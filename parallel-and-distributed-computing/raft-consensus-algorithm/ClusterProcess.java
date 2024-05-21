package cz.cvut.fel.agents.pdv.student;

import cz.cvut.fel.agents.pdv.dsand.Message;
import cz.cvut.fel.agents.pdv.dsand.Pair;
import cz.cvut.fel.agents.pdv.evaluation.StoreOperationEnums;
import cz.cvut.fel.agents.pdv.raft.RaftProcess;
import cz.cvut.fel.agents.pdv.raft.messages.*;

import java.util.*;
import java.util.function.BiConsumer;
import java.util.stream.Collectors;

import static cz.cvut.fel.agents.pdv.evaluation.StoreOperationEnums.*;

public class ClusterProcess extends RaftProcess<Map<String, String>> {

    private static final int TIMEOUT_MIN = 10;
    private static final int TIMEOUT_MAX = 20;
    private static final int UNDEFINED = -1;

    private final List<String> otherProcesses;
    private final int processCount;
    private final int networkDelay;

    private enum State {
        LEADER, FOLLOWER, CANDIDATE
    }

    private State state;
    private int term;
    private String leader;
    private String votedFor;
    private Set<String> votes;

    private int tick;
    private int heartbeatTick;
    private int timeout;

    // leader's attributes
    private final DatabaseLog log;
    private final Map<String, Integer> nextIndex;
    private final Map<String, Integer> matchIndex;
    private final Map<Integer, Pair<String, String>> notifyWithConfirm;

    private final Random random;

    public ClusterProcess(
            String id, Queue<Message> inbox, BiConsumer<String, Message> outbox,
            List<String> otherProcesses, int networkDelay
    ) {
        super(id, inbox, outbox);
        this.otherProcesses = otherProcesses;
        processCount = 1 + otherProcesses.size();
        this.networkDelay = Math.max(networkDelay, 1);
        random = new Random();

        state = State.FOLLOWER;
        term = 0;
        leader = null;
        votedFor = null;
        votes = new HashSet<>();

        tick = 0;
        heartbeatTick = 0;
        chooseRandomTimeout();

        log = new DatabaseLog();
        nextIndex = new HashMap<>();
        matchIndex = new HashMap<>();
        for (String process : otherProcesses) {
            nextIndex.put(process, log.size());
            matchIndex.put(process, 0);
        }
        notifyWithConfirm = new HashMap<>();
    }

    @Override
    public Optional<Map<String, String>> getLastSnapshotOfLog() {
        return Optional.of(log.toDatabase());
    }

    @Override
    public String getCurrentLeader() {
        return leader;
    }

    private void broadcast(Message msg) {
        for (String process : otherProcesses) {
            send(process, msg);
        }
    }

    private void chooseRandomTimeout() {
        timeout = (TIMEOUT_MIN + random.nextInt((TIMEOUT_MAX - TIMEOUT_MIN) + 1)) * networkDelay;
    }

    private void handleClientServer(ClientServerMessage msg) {
        if (msg instanceof ClientRequestWhoIsLeader) {
            send(msg.sender, new ServerResponseLeader(msg.getRequestId(), getCurrentLeader()));
        }
        else if (msg instanceof ClientRequestWithContent<?, ?>) {
            ClientRequestWithContent<StoreOperationEnums, Pair<String, String>> contentMsg =
                    (ClientRequestWithContent<StoreOperationEnums, Pair<String, String>>) msg;

            if (state == State.LEADER) {
                if (contentMsg.getOperation() == PUT || contentMsg.getOperation() == APPEND) {
                    if (!log.containsRequestId(msg.getRequestId())) {
                        log.add(term, msg.getRequestId(), contentMsg.getOperation().getName(), contentMsg.getContent());
                        notifyWithConfirm.put(log.size(), new Pair<>(msg.sender, msg.getRequestId()));
                    }
                }
                else if (contentMsg.getOperation() == GET) {
                    send(msg.sender, new ServerResponseWithContent<>(
                            msg.getRequestId(), log.toDatabase().get(contentMsg.getContent().getFirst())
                    ));
                }
            }
            else {
                send(msg.sender, new ServerResponseLeader(msg.getRequestId(), getCurrentLeader()));
            }
        }
    }

    private void handleRequestVote(RPC msg) {
        if (msg instanceof RequestVote) {
            RequestVote requestMsg = (RequestVote) msg;
            boolean voteGranted = false;

            if (state == State.FOLLOWER) {
                if (votedFor == null && term == requestMsg.getTerm()) {
                    if (log.size() == 0) {
                        voteGranted = true;
                    }
                    else if (requestMsg.getLastLogIndex() != UNDEFINED) {
                        int idx1 = log.size() - 1;
                        int idx2 = requestMsg.getLastLogIndex();
                        int term1 = log.getTermAt(idx1);
                        int term2 = requestMsg.getLastLogTerm();
                        voteGranted = term1 < term2 || term1 == term2 && idx1 <= idx2;
                    }

                    if (voteGranted) {
                        votedFor = msg.sender;
                    }
                }
            }

            send(msg.sender, new RequestVoteReply(term, voteGranted));
        }
        else if (msg instanceof RequestVoteReply) {
            RequestVoteReply replyMsg = (RequestVoteReply) msg;

            if (state == State.CANDIDATE) {
                if (replyMsg.voteGranted()) {
                    votes.add(msg.sender);

                    if (votes.size() > processCount / 2) {
                        state = State.LEADER;
                        leader = getId();

                        for (String process : otherProcesses) {
                            nextIndex.put(process, log.size());
                            matchIndex.put(process, 0);
                        }
                    }
                }
            }
        }
    }

    private void handleAppendEntries(RPC msg) {
        if (msg instanceof AppendEntries) {
            AppendEntries appendMsg = (AppendEntries) msg;

            boolean success = false;
            heartbeatTick = tick;
            int prevIndex = appendMsg.getPrevLogIndex();
            int prevTerm = appendMsg.getPrevLogTerm();
            int next = prevIndex != UNDEFINED ? prevIndex + 1 : UNDEFINED;

            if (state != State.LEADER) {
                if (term == appendMsg.getTerm()) {
                    state = State.FOLLOWER;
                    leader = msg.sender;

                    if (prevIndex == UNDEFINED || prevIndex < log.size() && log.getTermAt(prevIndex) == prevTerm) {
                        success = true;
                        log.clearFrom(prevIndex == UNDEFINED ? 0 : prevIndex + 1);
                        log.addAll(appendMsg.getEntries());
                        log.updateCommitIndex(appendMsg.getLeaderCommit());
                        next = log.size();
                    }
                    else {
                        if (prevIndex < log.size()) log.clearFrom(prevIndex);
                    }
                }
            }

            send(msg.sender, new AppendEntriesReply(term, success, next));
        }
        else if (msg instanceof AppendEntriesReply) {
            AppendEntriesReply replyMsg = (AppendEntriesReply) msg;

            if (state == State.LEADER) {
                if (term == msg.getTerm() && replyMsg.isSuccess()) {
                    nextIndex.put(msg.sender, replyMsg.getNextIndex());
                    matchIndex.put(msg.sender, Math.max(replyMsg.getNextIndex(), matchIndex.get(msg.sender)));
                    
                    // find maximum commit index which a majority of servers has
                    Optional<Map.Entry<Integer, Long>> maxMajorityIndex = matchIndex
                            .values().stream()
                            .collect(Collectors.groupingBy(i -> i, Collectors.counting()))
                            .entrySet()
                            .stream().filter(c -> c.getValue() > processCount / 2)
                            .max(Map.Entry.comparingByValue());
                    maxMajorityIndex.ifPresent(e -> log.updateCommitIndex(e.getKey()));
                }
                else {
                    nextIndex.put(msg.sender, Math.max(replyMsg.getNextIndex() - 1, 0));
                }
            }
        }
    }

    @Override
    public void act() {
        tick++;

        if (state == State.LEADER) {
            // broadcast heartbeats / log entries
            for (String process : otherProcesses) {
                int next = nextIndex.get(process);
                int prev = next - 1;
                boolean validPrev = prev >= 0 && !log.isEmpty();
                send(process, new AppendEntries(
                        term, validPrev ? prev : UNDEFINED, validPrev ? log.getTermAt(prev) : UNDEFINED, log.getEntriesFrom(next), log.getCommitIndex()
                ));
            }

            // send responses to the clients (about PUT or APPEND)
            Iterator<Map.Entry<Integer, Pair<String, String>>> iter = notifyWithConfirm.entrySet().iterator();
            while (iter.hasNext()) {
                Map.Entry<Integer, Pair<String, String>> entry = iter.next();
                if (entry.getKey() <= log.getCommitIndex()) {
                    send(entry.getValue().getFirst(), new ServerResponseConfirm(entry.getValue().getSecond()));
                    iter.remove();
                }
            }
        }
        else {
            // start new elections if needed
            if (tick - heartbeatTick > timeout) {
                heartbeatTick = tick;
                if (state == State.CANDIDATE) chooseRandomTimeout();

                state = State.CANDIDATE;
                term++;
                leader = null;
                votedFor = getId();

                votes = new HashSet<>();
                votes.add(votedFor);

                broadcast(new RequestVote(
                        term,
                        !log.isEmpty() ? log.size() - 1 : UNDEFINED,
                        !log.isEmpty() ? log.getTermAt(log.size() - 1) : UNDEFINED
                ));
            }
        }

        while (!inbox.isEmpty()) {
            Message msg = inbox.remove();

            if (msg instanceof ClientServerMessage) {
                handleClientServer((ClientServerMessage) msg);
            }
            else if (msg instanceof RPC) {
                RPC rpcMsg = (RPC) msg;

                if (term < rpcMsg.getTerm()) {
                    state = State.FOLLOWER;
                    term = rpcMsg.getTerm();
                    leader = null;
                    votedFor = null;
                }

                if (rpcMsg instanceof AppendEntries || rpcMsg instanceof AppendEntriesReply) {
                    handleAppendEntries(rpcMsg);
                }
                else if (rpcMsg instanceof RequestVote || rpcMsg instanceof RequestVoteReply) {
                    handleRequestVote(rpcMsg);
                }
            }
        }
    }
}
