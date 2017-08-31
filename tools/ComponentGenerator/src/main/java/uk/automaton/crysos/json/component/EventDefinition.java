package uk.automaton.crysos.json.component;

import com.google.gson.annotations.SerializedName;
import uk.automaton.crysos.json.SourceReference;
import uk.automaton.crysos.json.TypeReference;

public class EventDefinition {

    @SerializedName("sourceReference")
    public SourceReference sourceReference;

    @SerializedName("name")
    public String name;

    @SerializedName("type")
    public TypeReference type;

}
