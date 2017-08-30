package uk.automaton.visitor;

import uk.automaton.json.EnumDefinition;
import uk.automaton.json.SourceReference;

public interface EnumVisitor {

    void visitEnum(SourceReference sourceRef, String name, String qualifiedName);

    void visitValueDefinition(EnumDefinition.ValueDefinition valueDef);

}
