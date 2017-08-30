package uk.automaton.json;

import com.google.gson.annotations.SerializedName;
import uk.automaton.json.type.FieldDefinition;
import uk.automaton.visitor.ASTVisitor;
import uk.automaton.visitor.TypeVisitor;

import java.util.List;

public class TypeDefinition {

    @SerializedName("sourceReference")
    public SourceReference sourceReference;

    @SerializedName("name")
    public String name;

    @SerializedName("qualifiedName")
    public String qualifiedName;

    @SerializedName("enumDefinitions")
    public List<EnumDefinition> enumDefinitions;

    @SerializedName("typeDefinitions")
    public List<TypeDefinition> typeDefinitions;

    @SerializedName("fieldDefinitions")
    public List<FieldDefinition> fieldDefinitions;

    public void accept(ASTVisitor visitor) {
        TypeVisitor tv = visitor.visitTypeDefinition(name, qualifiedName);
        if (tv != null) {
            accept(tv);
        }
    }

    public void accept(TypeVisitor typeVisitor) {
        typeVisitor.visitType(sourceReference, name, qualifiedName);
        for (TypeDefinition type : typeDefinitions) {
            typeVisitor.visitTypeDefinition(type);
        }
        for (EnumDefinition ed : enumDefinitions) {
            typeVisitor.visitEnumDefinition(ed);
        }
        for (FieldDefinition fd : fieldDefinitions) {
            typeVisitor.visitFieldDefinition(fd);
        }
    }

    @Override
    public String toString() {
        return "TypeDefinition{" +
                "sourceReference=" + sourceReference +
                ", name='" + name + '\'' +
                ", qualifiedName='" + qualifiedName + '\'' +
                ", enumDefinitions=" + enumDefinitions +
                ", typeDefinitions=" + typeDefinitions +
                ", fieldDefinitions=" + fieldDefinitions +
                '}';
    }

    public String outputName() {
        return qualifiedName.replace(".", "::");
    }
}
