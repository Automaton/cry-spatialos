package uk.automaton.visitor;

import uk.automaton.json.EnumDefinition;
import uk.automaton.json.SourceReference;
import uk.automaton.json.TypeDefinition;
import uk.automaton.json.type.FieldDefinition;

public interface TypeVisitor {

    void visitType(SourceReference ref, String name, String qualifiedName);

    void visitTypeDefinition(TypeDefinition type);

    void visitFieldDefinition(FieldDefinition field);

    void visitEnumDefinition(EnumDefinition enumDef);
}
