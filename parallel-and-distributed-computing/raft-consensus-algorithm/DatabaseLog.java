package cz.cvut.fel.agents.pdv.student;

import cz.cvut.fel.agents.pdv.dsand.Pair;

import java.io.Serializable;
import java.util.*;

public class DatabaseLog implements Serializable {

    private List<LogEntry> log;
    private final Map<String, String> database;
    private int commitIndex;
    private int updateIndex;

    public DatabaseLog() {
        log = new ArrayList<>();
        database = new HashMap<>();
        commitIndex = 0;
        updateIndex = 0;
    }

    public int size() {
        return log.size();
    }

    public boolean isEmpty() {
        return log.isEmpty();
    }

    public void updateCommitIndex(int newCommitIndex) {
        commitIndex = Math.max(commitIndex, Math.min(newCommitIndex, log.size()));
    }

    public int getCommitIndex() {
        return commitIndex;
    }

    public int getTermAt(int index) {
        return log.get(index).getTerm();
    }

    public void add(int term, String requestId, String operation, Pair<String, String> content) {
        log.add(new LogEntry(term, requestId, operation, content.getFirst(), content.getSecond()));
    }

    public void addAll(List<LogEntry> entries) {
        log.addAll(entries);
    }

    public List<LogEntry> getEntriesFrom(int index) {
        return new ArrayList<>(log.subList(index, log.size()));
    }

    public void clearFrom(int index) {
        if (index >= commitIndex) log = log.subList(0, index);
    }

    public boolean containsRequestId(String id) {
        for (LogEntry entry : log) {
            if (Objects.equals(entry.getRequestId(), id)) return true;
        }
        return false;
    }

    public Map<String, String> toDatabase() {
        // synchronize database with committed log
        for (; updateIndex < commitIndex; updateIndex++) {
            LogEntry entry = log.get(updateIndex);
            switch (entry.getOperation()) {
                case "PUT": database.put(entry.getKey(), entry.getValue()); break;
                case "APPEND": database.put(entry.getKey(), database.getOrDefault(entry.getKey(), "").concat(entry.getValue())); break;
            }
        }

        return database;
    }

    /**
     * Function for debug, which applies all the log.
     */
    public Map<String, String> toUncommittedDatabase() {
        Map<String, String> uncommitted = new HashMap<>();

        for (LogEntry entry : log) {
            switch (entry.getOperation()) {
                case "PUT": uncommitted.put(entry.getKey(), entry.getValue()); break;
                case "APPEND": uncommitted.put(entry.getKey(), uncommitted.getOrDefault(entry.getKey(), "").concat(entry.getValue())); break;
            }
        }

        return uncommitted;
    }
}
