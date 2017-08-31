package uk.automaton.crysos.visitor;

import uk.automaton.crysos.json.SourceReference;
import uk.automaton.crysos.json.TypeReference;
import uk.automaton.crysos.json.component.CommandDefinition;
import uk.automaton.crysos.json.component.EventDefinition;

public interface ComponentVisitor {

    void visitComponent(SourceReference sourceRef, String name, String qualifiedName, int id);

    void visitDataDefinition(TypeReference typeRef);

    void visitEventDefinition(EventDefinition eventDef);

    void visitCommandDefinition(CommandDefinition commandDef);

}
