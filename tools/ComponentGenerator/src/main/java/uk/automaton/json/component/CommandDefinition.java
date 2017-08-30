package uk.automaton.json.component;

import com.google.gson.annotations.SerializedName;
import uk.automaton.json.SourceReference;
import uk.automaton.json.TypeReference;

public class CommandDefinition {

    @SerializedName("sourceReference")
    public SourceReference sourceReference;

    @SerializedName("name")
    public String name;

    @SerializedName("requestType")
    public TypeReference requestType;

    @SerializedName("responseType")
    public TypeReference responseType;
}
