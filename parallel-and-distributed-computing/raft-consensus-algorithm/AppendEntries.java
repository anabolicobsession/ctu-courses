package cz.cvut.fel.agents.pdv.student;

import java.util.List;

public class AppendEntries extends RPC {

    private final int prevLogIndex;
    private final int prevLogTerm;
    private final List<LogEntry> entries;
    private final int leaderCommit;

    public AppendEntries(int term, int prevLogIndex, int prevLogTerm, List<LogEntry> entries, int leaderCommit) {
        super(term);
        this.prevLogIndex = prevLogIndex;
        this.prevLogTerm = prevLogTerm;
        this.entries = entries;
        this.leaderCommit = leaderCommit;
    }

    public int getPrevLogIndex() {
        return prevLogIndex;
    }

    public int getPrevLogTerm() {
        return prevLogTerm;
    }

    public List<LogEntry> getEntries() {
        return entries;
    }

    public int getLeaderCommit() {
        return leaderCommit;
    }
}
