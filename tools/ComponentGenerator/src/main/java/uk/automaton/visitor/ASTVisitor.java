package uk.automaton.visitor;

import uk.automaton.json.ComponentDefinition;
import uk.automaton.json.EnumDefinition;
import uk.automaton.json.SourceReference;
import uk.automaton.json.TypeDefinition;

public interface ASTVisitor {

    void visit(SourceReference sourceRef, String fullPath, String canonicalName, String packageName);

    EnumVisitor visitEnumDefinition(String name, String qualifiedName);

    TypeVisitor visitTypeDefinition(String name, String qualifiedName);

    ComponentVisitor visitComponentDefinition(String name, String qualifiedName, int id);

}
