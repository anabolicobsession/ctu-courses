package cz.cvut.fel.agents.pdv.student;

import cz.cvut.fel.agents.pdv.dsand.Message;

import java.io.Serializable;

public class RPC extends Message {

    private final int term;

    public RPC(int term) {
        this.term = term;
    }

    public int getTerm() {
        return term;
    }
}
