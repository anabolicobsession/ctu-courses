package cz.cvut.fel.agents.pdv.swim;

import cz.cvut.fel.agents.pdv.dsand.Message;
import cz.cvut.fel.agents.pdv.dsand.Pair;

import java.util.*;
import java.util.stream.Collectors;

public class ActStrategy {
    private final int maxDelayForMessages;
    private final List<String> otherProcesses;

    private int time;
    private final Random random;

    private int pingTime;
    private String pingedProcess;
    private boolean ackReceived;

    boolean pingRequestSent;
    private int numberOfPingRequestedProcesses;
    List<String> pingRequestedProcesses;
    PingedByRequest pingedByRequest;

    public ActStrategy(int maxDelayForMessages, List<String> otherProcesses, int timeToDetectKilledProcess, int upperBoundOnMessages) {
        this.maxDelayForMessages = maxDelayForMessages;
        this.otherProcesses = otherProcesses;

        time = 0;
        random = new Random();

        pingTime = Integer.MAX_VALUE;
        ackReceived = false;

        pingRequestSent = false;
        numberOfPingRequestedProcesses = 7;
        pingRequestedProcesses = new ArrayList<String>();
        pingedByRequest = new PingedByRequest(2 * maxDelayForMessages);
    }

    public List<Pair<String, Message>> act(Queue<Message> inbox, String disseminationProcess) {
        List<Pair<String, Message>> outbox = new ArrayList<>();
        boolean pingNewProcess = false;

        while (!inbox.isEmpty()) {
            Message message = inbox.remove();

            if (message instanceof PingMessage) {
                PingMessage ping = (PingMessage) message;
                outbox.add(new Pair<>(ping.sender, new AckMessage()));
            }
            else if (message instanceof AckMessage) {
                AckMessage ack = (AckMessage) message;

                if (ack.sender.equals(pingedProcess) || pingRequestedProcesses.contains(ack.sender)) {
                    ackReceived = true;
                }

                if (pingedByRequest.wasPingedByRequest(ack.sender)) {
                    for (String pingRequested : pingedByRequest.getRequestedPingTo(ack.sender)) {
                        outbox.add(new Pair<>(pingRequested, new AckMessage()));
                    }
                    pingedByRequest.removePingedProcess(ack.sender);
                }
            }
            else if (message instanceof PingRequestMessage) {
                PingRequestMessage pingRequest = (PingRequestMessage) message;
                pingedByRequest.add(pingRequest.sender, pingRequest.getProcessToPing(), time);
                outbox.add(new Pair<>(pingRequest.getProcessToPing(), new PingMessage()));
            }
            else if (message instanceof DeadProcessMessage) {
                DeadProcessMessage deadProcess = (DeadProcessMessage) message;
                otherProcesses.remove(deadProcess.getProcessID());
            }
            else {
                throw new IllegalStateException("Unknown message type");
            }
        }

        if (time == 0 || ackReceived) {
            pingNewProcess = true;
        }
        else if (time - pingTime >= 2 * maxDelayForMessages && !pingRequestSent) {
            pingRequestSent = true;
            pingRequestedProcesses = new ArrayList<>();
            numberOfPingRequestedProcesses = Math.min(numberOfPingRequestedProcesses, otherProcesses.size());
            List<String> otherProcessesCopy = new ArrayList<>(otherProcesses);

            otherProcessesCopy.remove(pingedProcess);
            otherProcessesCopy.removeIf(process -> pingedByRequest.wasPingedByRequest(process));
            List<String> recipients = outbox.stream().map(Pair::getFirst).collect(Collectors.toList());
            otherProcessesCopy.removeIf(recipients::contains);

            if (otherProcessesCopy.size() < numberOfPingRequestedProcesses - pingRequestedProcesses.size()) {
                throw new IllegalStateException("Not enough processes for ping request");
            }

            while (pingRequestedProcesses.size() != numberOfPingRequestedProcesses) {
                int idx = random.nextInt(otherProcessesCopy.size());
                pingRequestedProcesses.add(otherProcessesCopy.get(idx));
                otherProcessesCopy.remove(idx);
            }

            for (String process : pingRequestedProcesses) {
                outbox.add(new Pair<>(process, new PingRequestMessage(pingedProcess)));
            }
        }
        else if (time - pingTime >= 6 * maxDelayForMessages) {
            outbox.add(new Pair<>(disseminationProcess, new PFDMessage(pingedProcess)));
            pingNewProcess = true;
        }

        if (pingNewProcess) {
            pingTime = time;
            ackReceived = false;
            pingRequestSent = false;
            pingRequestedProcesses = new ArrayList<>();

            String oldPingedProcess = pingedProcess;
            while (Objects.equals(pingedProcess, oldPingedProcess)) {
                pingedProcess = otherProcesses.get(random.nextInt(otherProcesses.size()));
            }
            outbox.add(new Pair<>(pingedProcess, new PingMessage()));
        }

        time++;
        pingedByRequest.update(time);

        return outbox;
    }
}
