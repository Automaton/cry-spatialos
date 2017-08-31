package uk.automaton.crysos.json;

import com.google.gson.annotations.SerializedName;
import uk.automaton.crysos.json.component.CommandDefinition;
import uk.automaton.crysos.json.component.EventDefinition;
import uk.automaton.crysos.visitor.ASTVisitor;
import uk.automaton.crysos.visitor.ComponentVisitor;

import java.util.List;

public class ComponentDefinition {

    @SerializedName("sourceReference")
    public SourceReference sourceReference;

    @SerializedName("name")
    public String name;

    @SerializedName("qualifiedName")
    public String qualifiedName;

    @SerializedName("id")
    public int id;

    @SerializedName("dataDefinition")
    public TypeReference dataDefinition;

    @SerializedName("eventDefinitions")
    public List<EventDefinition> eventDefinitions;

    @SerializedName("commandDefinitions")
    public List<CommandDefinition> commandDefinitions;

    public void accept(ASTVisitor visitor) {
        ComponentVisitor cv = visitor.visitComponentDefinition(name, qualifiedName, id);
        if (cv != null) {
            accept(cv);
        }
    }

    public void accept(ComponentVisitor cv) {
        cv.visitComponent(sourceReference, name, qualifiedName, id);
        for (EventDefinition ed : eventDefinitions) {
            cv.visitEventDefinition(ed);
        }
        for (CommandDefinition cd : commandDefinitions) {
            cv.visitCommandDefinition(cd);
        }
        cv.visitDataDefinition(dataDefinition);
    }
}
