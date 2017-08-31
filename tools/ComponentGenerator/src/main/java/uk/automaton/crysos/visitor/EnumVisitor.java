package uk.automaton.crysos.visitor;

import uk.automaton.crysos.json.EnumDefinition;
import uk.automaton.crysos.json.SourceReference;

public interface EnumVisitor {

    void visitEnum(SourceReference sourceRef, String name, String qualifiedName);

    void visitValueDefinition(EnumDefinition.ValueDefinition valueDef);

}
