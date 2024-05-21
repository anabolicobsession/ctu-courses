package cz.cvut.fel.agents.pdv.student;

public class RequestVoteReply extends RPC {

    private final boolean voteGranted;

    public RequestVoteReply(int term, boolean voteGranted) {
        super(term);
        this.voteGranted = voteGranted;
    }

    public boolean voteGranted() {
        return voteGranted;
    }
}
