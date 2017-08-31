package uk.automaton.crysos.json;

import com.google.gson.annotations.SerializedName;
import uk.automaton.crysos.visitor.ASTVisitor;
import uk.automaton.crysos.visitor.EnumVisitor;

import java.util.List;

public class EnumDefinition {

    public static class ValueDefinition {

        @SerializedName("sourceReference")
        public SourceReference sourceReference;

        @SerializedName("name")
        public String name;

        @SerializedName("value")
        public int value;

    }

    @SerializedName("sourceReference")
    public SourceReference sourceReference;

    @SerializedName("name")
    public String name;

    @SerializedName("qualifiedName")
    public String qualifiedName;

    @SerializedName("valueDefinitions")
    public List<ValueDefinition> valueDefinitions;

    public void accept(ASTVisitor visitor) {
        EnumVisitor ev = visitor.visitEnumDefinition(name, qualifiedName);
        if (ev != null) {
            accept(ev);
        }
    }

    public void accept(EnumVisitor ev) {
        ev.visitEnum(sourceReference, name, qualifiedName);
        for (ValueDefinition vd : valueDefinitions) {
            ev.visitValueDefinition(vd);
        }
    }

}
