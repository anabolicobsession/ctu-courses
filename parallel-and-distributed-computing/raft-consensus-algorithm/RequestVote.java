package cz.cvut.fel.agents.pdv.student;

public class RequestVote extends RPC {

    private final int lastLogIndex;
    private final int lastLogTerm;

    public RequestVote(int term, int lastLogIndex, int lastLogTerm) {
        super(term);
        this.lastLogIndex = lastLogIndex;
        this.lastLogTerm = lastLogTerm;
    }

    public int getLastLogIndex() {
        return lastLogIndex;
    }

    public int getLastLogTerm() {
        return lastLogTerm;
    }
}
