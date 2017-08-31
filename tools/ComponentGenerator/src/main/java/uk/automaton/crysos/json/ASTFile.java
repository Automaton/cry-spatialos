package uk.automaton.crysos.json;

import com.google.gson.annotations.SerializedName;
import uk.automaton.crysos.visitor.ASTVisitor;

import java.util.List;

public class ASTFile {

    @SerializedName("sourceReference")
    public SourceReference sourceReference;

    @SerializedName("completePath")
    public String completePath;

    @SerializedName("canonicalName")
    public String canonicalName;

    @SerializedName("package")
    public String packageName;

    @SerializedName("enumDefinitions")
    public List<EnumDefinition> enumDefinitions;

    @SerializedName("typeDefinitions")
    public List<TypeDefinition> typeDefinitions;

    @SerializedName("componentDefinitions")
    public List<ComponentDefinition> componentDefinitions;

    public void accept(ASTVisitor visitor) {
        visitor.visit(sourceReference, completePath, canonicalName, packageName);
        for (EnumDefinition enumDefinition : enumDefinitions) {
            enumDefinition.accept(visitor);
        }
        for (TypeDefinition typeDefinition : typeDefinitions) {
            typeDefinition.accept(visitor);
        }
        for (ComponentDefinition componentDefinition : componentDefinitions) {
            componentDefinition.accept(visitor);
        }
    }

}
