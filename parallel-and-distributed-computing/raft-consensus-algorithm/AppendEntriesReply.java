package cz.cvut.fel.agents.pdv.student;

public class AppendEntriesReply extends RPC {

    private final boolean success;
    private final int nextIndex;

    public AppendEntriesReply(int term, boolean success, int nextIndex) {
        super(term);
        this.success = success;
        this.nextIndex = nextIndex;
    }

    public boolean isSuccess() {
        return success;
    }

    public int getNextIndex() {
        return nextIndex;
    }
}
