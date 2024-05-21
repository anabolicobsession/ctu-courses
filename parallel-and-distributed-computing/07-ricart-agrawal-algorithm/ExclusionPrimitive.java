package cz.cvut.fel.agents.pdv.exclusion;

import cz.cvut.fel.agents.pdv.clocked.ClockedMessage;
import cz.cvut.fel.agents.pdv.clocked.ClockedProcess;

import java.util.*;

public class ExclusionPrimitive {
    enum AcquisitionState {
        RELEASED, WANTED, HELD
    }

    private final ClockedProcess owner;
    private final String criticalSectionName;
    private final List<String> allAccessingProcesses;
    private AcquisitionState state;

    private int requestTime;
    private final Map<String, Boolean> receivedOKFrom;
    private final List<String> requestedProcesses;

    public ExclusionPrimitive(ClockedProcess owner, String criticalSectionName, String[] allProcesses) {
        this.owner = owner;
        this.criticalSectionName = criticalSectionName;
        this.allAccessingProcesses = new ArrayList<>(Arrays.asList(allProcesses));
        this.state = AcquisitionState.RELEASED;

        requestTime = Integer.MAX_VALUE;
        receivedOKFrom = new HashMap<>();
        resetReceivedOKFrom();
        requestedProcesses = new ArrayList<>();
    }

    private void switchState() {
        state = AcquisitionState.values()[(state.ordinal() + 1) % AcquisitionState.values().length];
    }

    private boolean isLessThan(String s1, String s2) {
        return s1.length() == s2.length() ? s1.compareTo(s2) < 0 : s1.length() < s2.length();
    }

    private void resetReceivedOKFrom() {
        for (String process : allAccessingProcesses) {
            if (!Objects.equals(process, owner.id)) {
                receivedOKFrom.put(process, false);
            }
        }
    }

    private boolean isReceivedOKFromAll() {
        for (Map.Entry<String, Boolean> entry : receivedOKFrom.entrySet()) {
            if (!entry.getValue()) {
                return false;
            }
        }

        return true;
    }

    private void send(String process, ClockedMessage message) {
        owner.increaseTime();
        if (!Objects.equals(process, owner.id)) {
            owner.send(process, message);
        }
    }

    private void send(List<String> processes, ClockedMessage message) {
        owner.increaseTime();
        for (String process : processes) {
            if (!Objects.equals(process, owner.id)) {
                owner.send(process, message);
            }
        }
    }

    public boolean accept(ClockedMessage message) {
        String sender = message.sender;

        if (message instanceof ExclusionMessage && Objects.equals(((ExclusionMessage) message).getCriticalSectionName(), criticalSectionName)) {
            if (message instanceof RequestMessage) {
                if (state == AcquisitionState.HELD || (state == AcquisitionState.WANTED
                        && (requestTime < message.sentOn || (requestTime == message.sentOn && isLessThan(owner.id, sender))))) {
                    requestedProcesses.add(sender);
                } else {
                    send(sender, new OKMessage(criticalSectionName));
                }
            }
            else if (message instanceof OKMessage) {
                receivedOKFrom.put(sender, true);

                if (isReceivedOKFromAll()) {
                    this.owner.increaseTime();
                    switchState();
                }
            }

            return true;
        }

        return false;
    }

    public void requestEnter() {
        switchState();
        resetReceivedOKFrom();
        send(allAccessingProcesses, new RequestMessage(criticalSectionName));
        requestTime = owner.getTime();
    }

    public void exit() {
        switchState();
        send(requestedProcesses, new OKMessage(criticalSectionName));
        requestedProcesses.clear();
    }

    public String getName() {
        return criticalSectionName;
    }

    public boolean isHeld() {
        return state == AcquisitionState.HELD;
    }
}
