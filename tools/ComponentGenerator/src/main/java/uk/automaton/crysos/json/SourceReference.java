package uk.automaton.crysos.json;

import com.google.gson.annotations.SerializedName;

public class SourceReference {

    @SerializedName("line")
    public int line;

    @SerializedName("column")
    public int column;

    @Override
    public String toString() {
        return "SourceReference{" +
                "line=" + line +
                ", column=" + column +
                '}';
    }
}
