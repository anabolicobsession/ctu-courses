package cz.cvut.fel.agents.pdv.swim;

import cz.cvut.fel.agents.pdv.dsand.Pair;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public class PingedByRequest {
    private final List<List<Pair<String, String>>> pingedByRequest;

    public PingedByRequest(int size) {
        pingedByRequest = new ArrayList<>();
        for (int i = 0; i < size; i++) {
            pingedByRequest.add(new ArrayList<>());
        }
    }

    public void add(String requestedPing, String pingedProcess, int time) {
        pingedByRequest.get(time % pingedByRequest.size()).add(new Pair<>(requestedPing, pingedProcess));
    }

    public void update(int time) {
        pingedByRequest.get(time % pingedByRequest.size()).clear();
    }

    public boolean wasPingedByRequest(String process) {
        for (List<Pair<String, String>> list : pingedByRequest) {
            for (Pair<String, String> request : list) {
                if (Objects.equals(request.getSecond(), process)) {
                    return true;
                }
            }
        }

        return false;
    }

    public List<String> getRequestedPingTo(String process) {
        List<String> requestedPing = new ArrayList<>();

        for (List<Pair<String, String>> list : pingedByRequest) {
            for (Pair<String, String> request : list) {
                if (Objects.equals(request.getSecond(), process)) {
                    requestedPing.add(request.getFirst());
                }
            }
        }

        return requestedPing;
    }

    public void removePingedProcess(String process) {
        pingedByRequest.forEach(list -> list.removeIf(request -> Objects.equals(request.getSecond(), process)));
    }
}
