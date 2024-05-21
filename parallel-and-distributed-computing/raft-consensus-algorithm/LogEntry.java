package cz.cvut.fel.agents.pdv.student;

import java.io.Serializable;

public class LogEntry implements Serializable {

    private final int term;
    private final String requestId;
    private final String operation;
    private final String key;
    private final String value;

    public LogEntry(int term, String requestId, String operation, String key, String value) {
        this.term = term;
        this.requestId = requestId;
        this.operation = operation;
        this.key = key;
        this.value = value;
    }

    public int getTerm() {
        return term;
    }

    public String getRequestId() {
        return requestId;
    }

    public String getOperation() {
        return operation;
    }

    public String getKey() {
        return key;
    }

    public String getValue() {
        return value;
    }

    @Override
    public String toString() {
        return term + "=" + String.join("/", key, value);
    }
}
