package uk.automaton.crysos.visitor;

import uk.automaton.crysos.json.SourceReference;

public interface ASTVisitor {

    void visit(SourceReference sourceRef, String fullPath, String canonicalName, String packageName);

    EnumVisitor visitEnumDefinition(String name, String qualifiedName);

    TypeVisitor visitTypeDefinition(String name, String qualifiedName);

    ComponentVisitor visitComponentDefinition(String name, String qualifiedName, int id);

}
