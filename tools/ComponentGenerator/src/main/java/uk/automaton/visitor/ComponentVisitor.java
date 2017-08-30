package uk.automaton.visitor;

import uk.automaton.json.SourceReference;
import uk.automaton.json.TypeReference;
import uk.automaton.json.component.CommandDefinition;
import uk.automaton.json.component.EventDefinition;

public interface ComponentVisitor {

    void visitComponent(SourceReference sourceRef, String name, String qualifiedName, int id);

    void visitDataDefinition(TypeReference typeRef);

    void visitEventDefinition(EventDefinition eventDef);

    void visitCommandDefinition(CommandDefinition commandDef);

}
