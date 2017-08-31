package uk.automaton.crysos.visitor;

import uk.automaton.crysos.json.EnumDefinition;
import uk.automaton.crysos.json.SourceReference;
import uk.automaton.crysos.json.TypeDefinition;
import uk.automaton.crysos.json.type.FieldDefinition;

public interface TypeVisitor {

    void visitType(SourceReference ref, String name, String qualifiedName);

    void visitTypeDefinition(TypeDefinition type);

    void visitFieldDefinition(FieldDefinition field);

    void visitEnumDefinition(EnumDefinition enumDef);
}
