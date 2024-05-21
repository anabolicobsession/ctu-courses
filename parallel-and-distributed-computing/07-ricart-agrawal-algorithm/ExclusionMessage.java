package cz.cvut.fel.agents.pdv.exclusion;

import cz.cvut.fel.agents.pdv.clocked.ClockedMessage;

public class ExclusionMessage extends ClockedMessage {
    private final String criticalSectionName;

    public ExclusionMessage(String criticalSectionName) {
        this.criticalSectionName = criticalSectionName;
    }

    public String getCriticalSectionName() {
        return criticalSectionName;
    }
}
