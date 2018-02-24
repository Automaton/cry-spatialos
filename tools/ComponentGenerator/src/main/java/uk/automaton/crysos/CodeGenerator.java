package uk.automaton.crysos;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.stringtemplate.v4.ST;
import org.stringtemplate.v4.STGroup;
import org.stringtemplate.v4.STGroupFile;
import uk.automaton.crysos.json.*;
import uk.automaton.crysos.json.component.CommandDefinition;
import uk.automaton.crysos.json.component.EventDefinition;
import uk.automaton.crysos.json.type.FieldDefinition;
import uk.automaton.crysos.visitor.ASTVisitor;
import uk.automaton.crysos.visitor.ComponentVisitor;
import uk.automaton.crysos.visitor.EnumVisitor;
import uk.automaton.crysos.visitor.TypeVisitor;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;
import java.util.stream.Collectors;

public class CodeGenerator {

    private static final Logger logger = LoggerFactory.getLogger(CodeGenerator.class);

    private final TypeMapper typeMapper;
    private final GuidManager guidManager;

    public CodeGenerator(TypeMapper mapper, GuidManager manager) {
        this.typeMapper = mapper;
        this.guidManager = manager;
    }

    static String upperName(String name) {
        if (name.length() == 0) {
            throw new IllegalArgumentException("name should not be empty");
        }
        String ret = name;
        if (Character.isLowerCase(name.charAt(0))) {
            ret = Character.toUpperCase(name.charAt(0)) + name.substring(1);
        }
        // Remove _ and replace with camelcase
        StringBuilder output = new StringBuilder();
        boolean upperNext = false;
        for (int i = 0; i < ret.length(); i++) {
            char c = ret.charAt(i);
            if (c == '_') {
                upperNext = true;
                continue;
            }
            output.append(upperNext ? Character.toUpperCase(c) : c);
            upperNext = false;
        }
        return output.toString();
    }

    static String formatGuid(UUID uuid) {
        return "{" + uuid.toString().toUpperCase() + "}";
    }


    public void generateHeader(ASTFile file, Path outputPath) {
        final List<GenerateComponentHeader> generateComponentHeaders = new LinkedList<>();
        file.accept(new ASTVisitor() {
            String compName;

            @Override
            public void visit(SourceReference sourceRef, String fullPath, String canonicalName, String packageName) {
                compName = canonicalName.replace(".schema", ".h");
            }

            @Override
            public EnumVisitor visitEnumDefinition(String name, String qualifiedName) {
                return null;
            }

            @Override
            public TypeVisitor visitTypeDefinition(String name, String qualifiedName) {
                return null;
            }

            @Override
            public ComponentVisitor visitComponentDefinition(String name, String qualifiedName, int id) {
                GenerateComponentHeader v = new GenerateComponentHeader(compName);
                logger.trace("Generating header code for {}", qualifiedName);
                generateComponentHeaders.add(v);
                return v;
            }
        });

        for (GenerateComponentHeader gch : generateComponentHeaders) {
            gch.writeToFile(outputPath);
        }
    }

    public GenerateSchematyc generateSchematyc(ASTFile file) {
        GenerateSchematyc gen = new GenerateSchematyc();
        file.accept(gen);
        return gen;
    }

    class ReflectTypeGenerator implements TypeVisitor {

        private String typeClass;
        private String guid;
        private String name;

        private String qualifiedName;

        private STGroupFile group;
        private List<String> funcDefinitions;

        public ReflectTypeGenerator() {
            group = new STGroupFile("templates/SchematycTemplate.stg");
            funcDefinitions = new LinkedList<>();
        }

        @Override
        public void visitType(SourceReference ref, String name, String qualifiedName) {
            this.qualifiedName = qualifiedName;
            typeClass = qualifiedName.replace(".", "::");
            guid = formatGuid(guidManager.getOrCreateTypeGuid(qualifiedName));
            this.name = upperName(name);
        }

        @Override
        public void visitTypeDefinition(TypeDefinition type) {
            logger.warn("ComponentGenerator does not currently support nested type definitions");
        }

        @Override
        public void visitFieldDefinition(FieldDefinition field) {
            // SpatialOS generated fields are private, so need to generate getters/setters for schematyc
            String getterName = field.name;
            String setterName = "set_" + field.name;

            if (field.getFieldType() != FieldDefinition.FieldType.SINGULAR) {
                logger.warn("Cannot generate Schematyc functions for {}, only singular type is currently supported", field.name);
                return;
            }
            String resolvedType = typeMapper.resolve(field.singularType);

            if (resolvedType.equals("double") || resolvedType.equals("worker::EntityId") || resolvedType.equals("std::string")) {
                logger.warn("Cannot generate Schematyc functions for {} type {}, type not supported", field.name, resolvedType);
                return;
            }
            {
                // generate getter
                String guid = formatGuid(guidManager.getOrCreateTypeGuid(qualifiedName + "." + getterName));
                String className = typeClass;
                String output = upperName(field.name);
                ST st = group.getInstanceOf("getterFunc");
                st.add("className", className);
                st.add("funcName", getterName);
                st.add("label", upperName("get_" + getterName));
                st.add("guid", guid);
                st.add("output", output);
                StringBuilder fnPtr = new StringBuilder();

                if (resolvedType.contains("::") && !resolvedType.equals("worker::EntityId")) {
                    fnPtr.append("const ::");
                    fnPtr.append(resolvedType).append("& (");
                } else {
                    fnPtr.append(resolvedType).append(" (");
                }

                fnPtr.append(className).append("::*)() const");
                st.add("ptrType", fnPtr.toString());
                funcDefinitions.add(st.render());
            }
            {
                // generate setter
                String guid = formatGuid(guidManager.getOrCreateTypeGuid(qualifiedName + "." + setterName));
                String className = typeClass;
                String input = upperName(field.name);
                ST st = group.getInstanceOf("setterFunc");
                st.add("className", className);
                st.add("funcName", setterName);
                st.add("label", upperName(setterName));
                st.add("guid", guid);
                st.add("input", input);
                st.add("defaultVal", typeMapper.defaultValue(field));
                funcDefinitions.add(st.render());
            }
        }

        @Override
        public void visitEnumDefinition(EnumDefinition enumDef) {

        }

        public String getFunctionSignature() {
            return getFunctionBody();
        }

        public String getFunctionBody() {
            String[] namespaces = qualifiedName.split("\\.");
            StringBuilder sb = new StringBuilder();
            if (namespaces.length > 1) {
                for (int i = 0; i < namespaces.length - 1; ++i) {
                    sb.append("namespace ").append(namespaces[i]).append(" { ");
                }
                sb.append('\n');
            }
            ST st = group.getInstanceOf("reflectTypeBody");
            st.add("type", typeClass);
            st.add("guid", guid);
            st.add("name", name);
            sb.append(st.render());
            sb.append('\n');
            if (namespaces.length > 1)
                for (int i = 0; i < namespaces.length - 1; ++i) {
                    sb.append('}');
                }
            return sb.toString();
        }

        public String getRegistration() {
            ST st = group.getInstanceOf("scopeReg");
            st.add("typeName", typeClass);
            String funcDefs = funcDefinitions.stream().collect(Collectors.joining("\n"));
            st.add("reg", funcDefs);
            return st.render();
        }
    }

    class GenerateSchematyc implements ASTVisitor {

        private String includes = "";
        private List<ReflectTypeGenerator> reflectTypes = new LinkedList<>();

        @Override
        public void visit(SourceReference sourceRef, String fullPath, String canonicalName, String packageName) {
            includes += "#include <" + canonicalName.replace(".schema", ".h") + ">";
        }

        @Override
        public EnumVisitor visitEnumDefinition(String name, String qualifiedName) {
            return null;
        }

        @Override
        public TypeVisitor visitTypeDefinition(String name, String qualifiedName) {
            TypeDefinition typeDef = typeMapper.getTypeDef(qualifiedName);
            if (typeDef == null) {
                logger.warn("Failed to resolve type for {}", qualifiedName);
                return null;
            }
            if (typeMapper.isComponentType(typeDef)) {
                logger.trace("Skipping {}", qualifiedName);
                return null;
            }
            ReflectTypeGenerator rtg = new ReflectTypeGenerator();
            reflectTypes.add(rtg);
            return rtg;
        }

        @Override
        public ComponentVisitor visitComponentDefinition(String name, String qualifiedName, int id) {
            return null;
        }

        public String getIncludes() {
            return includes;
        }

        public String getSignatures() {
            return reflectTypes.stream().map(ReflectTypeGenerator::getFunctionSignature).collect(Collectors.joining("\n"));
        }

        public String getFunctionReg() {
            return reflectTypes.stream().map(ReflectTypeGenerator::getRegistration).collect(Collectors.joining("\n"));
        }

        public String getFunctionBodies() {
            //return reflectTypes.stream().map(ReflectTypeGenerator::getFunctionBody).collect(Collectors.joining("\n"));
            return "";
        }
    }


    class GenerateComponentHeader implements ComponentVisitor {

        private String className = "";
        private String componentClassName = "";
        private String componentHeader = "";

        private String outputFileName = "";
        private String fields = "";
        private String functions = "";
        private String schematyc = "";
        private String includes = "";
        private String componentUpperName = "";
        private String reflectFields = "";
        private String updateFields = "";
        private String initialiseBody = "";

        private String schematycFuncs = "";
        private String schematycSignals = "";
        private String componentFields = "";
        private String constructorFields = "";

        private int componentId;

        private final STGroup group;


        public GenerateComponentHeader(String componentHeader) {
            this.componentHeader = componentHeader;
            group = new STGroupFile("templates/HeaderTemplate.stg");
        }

        @Override
        public void visitComponent(SourceReference sourceRef, String name, String qualifiedName, int id) {
            className = "CS" + upperName(name);
            componentUpperName = upperName(name);
            componentClassName = qualifiedName.replace(".", "::");
            outputFileName = "components/" + qualifiedName.replace(".", "/") + ".h";
            componentId = id;
        }

        @Override
        public void visitDataDefinition(TypeReference typeRef) {
            assert (typeRef.builtInType == null);

            TypeDefinition typeDef = typeMapper.getTypeDef(typeRef.getType());
            if (typeDef == null) {
                logger.error("Failed to find type definition for {}", typeRef.userType);
                return;
            }

            StringBuilder fields = new StringBuilder();
            StringBuilder functions = new StringBuilder();
            StringBuilder callbackFields = new StringBuilder();
            StringBuilder callbackFunctions = new StringBuilder();
            StringBuilder schematycSignals = new StringBuilder();
            StringBuilder additionalIncludes = new StringBuilder();
            StringBuilder reflectFields = new StringBuilder();
            StringBuilder updateFields = new StringBuilder();
            Set<String> includesSet = new TreeSet<>();

            StringBuilder schematycFuncReg = new StringBuilder();
            StringBuilder schematycSigReg = new StringBuilder();
            StringBuilder componentFields = new StringBuilder();
            StringBuilder constructorFields = new StringBuilder();

            for (FieldDefinition fieldDef : typeDef.fieldDefinitions) {
                String fieldType = typeMapper.getOutputType(fieldDef);
                String fieldName = "m_" + fieldDef.name;
                String upperName = upperName(fieldDef.name);
                logger.trace("Processing field {}", fieldDef);

                for (TypeReference r : fieldDef.getReferences()) {
                    if (!r.isBuiltIn()) {
                        TypeDefinition def = typeMapper.getTypeDef(r.userType);
                        if (def == null) {
                            logger.warn("Failed to resolve user type {} for building includes", r.userType);
                            continue;
                        }
                        String loc = typeMapper.getLocation(def);
                        if (loc == null) {
                            logger.warn("Failed to resolve location for user type: {}", def);
                            continue;
                        }
                        includesSet.add(loc);
                    }
                }
                if (componentFields.length() != 0) {
                    componentFields.append(", ");
                }
                if (constructorFields.length() != 0) {
                    constructorFields.append(", ");
                } else {
                    constructorFields.append(": ");
                }
                componentFields.append(fieldName);
                constructorFields.append(fieldName).append("(").append(typeMapper.defaultValue(fieldDef)).append(")");
                ST st = group.getInstanceOf("fieldTemplate");
                st.add("name", fieldName);
                st.add("type", fieldType);
                fields.append(st.render()).append('\n');
                st = group.getInstanceOf("getterTemplate");
                st.add("name", upperName);
                st.add("fieldName", fieldName);
                st.add("type", fieldType);
                functions.append(st.render()).append('\n');
                st = group.getInstanceOf("setterTemplate");
                st.add("name", upperName);
                st.add("type", fieldType);
                st.add("fieldName", fieldDef.name);
                functions.append(st.render()).append('\n');
                st = group.getInstanceOf("callbackFieldTemplate");
                st.add("callbackName", fieldName + "_callbacks");
                st.add("types", fieldType);
                callbackFields.append(st.render()).append('\n');
                st = group.getInstanceOf("callbackFunctionTemplate");
                st.add("callbackName", upperName);
                st.add("types", fieldType);
                st.add("fieldName", fieldName + "_callbacks");
                callbackFunctions.append(st.render()).append('\n');
                st = group.getInstanceOf("updateField");
                st.add("spatialFieldName", fieldDef.name);
                st.add("fieldName", fieldName);
                updateFields.append(st.render()).append('\n');
                if (fieldType.equals("double") || fieldType.equals("worker::EntityId") || fieldType.equals("std::string") || fieldType.startsWith("worker::List") || fieldType.startsWith("worker::Map")) {
                    logger.warn("Cannot generate Schematyc functions for {} type {}, type not supported", fieldName, fieldType);
                } else {
                    st = group.getInstanceOf("signalStructTemplate");
                    st.add("name", componentUpperName + upperName);
                    st.add("type", fieldType);
                    st.add("field", fieldDef.name);
                    schematycSignals.append(st.render()).append('\n');
                    st = group.getInstanceOf("schematycSignal");
                    st.add("signalName", "S" + componentUpperName + upperName + "UpdatedSignal");
                    schematycSigReg.append(st.render()).append('\n');
                    st = group.getInstanceOf("signalReflectType");
                    st.add("name", componentUpperName + upperName);
                    st.add("field", fieldDef.name);
                    st.add("type", fieldType);
                    st.add("guid", formatGuid(guidManager.getOrCreateGuidSignal(componentId, fieldDef.number)));
                    st.add("id", fieldDef.number);
                    st.add("defaultVal", typeMapper.defaultValue(fieldDef));
                    schematycSignals.append(st.render()).append('\n');
                    st = group.getInstanceOf("reflectField");
                    st.add("clazzName", className);
                    st.add("name", fieldName);
                    st.add("defaultVal", typeMapper.defaultValue(fieldDef));
                    st.add("num", fieldDef.number);
                    st.add("upperName", upperName);
                    reflectFields.append(st.render()).append('\n');
                    st = group.getInstanceOf("updateSchematycObject");
                    st.add("fieldName", fieldName);
                    st.add("signalName", componentUpperName + upperName);
                    st = group.getInstanceOf("schematycFunc");
                    st.add("className", className);
                    st.add("funcName", "Update" + upperName);
                    st.add("guid", formatGuid(guidManager.getOrCreateGuidFunction(componentId, fieldDef.number)));
                    st.add("input", upperName);
                    schematycFuncReg.append(st.render()).append('\n');
                }

            }

            for (String include : includesSet) {
                if (include.equals(componentHeader)) continue;
                additionalIncludes.append("#include <" ).append(include).append(">").append("\n");
            }

            fields.append(callbackFields);
            functions.append(callbackFunctions);
            this.fields += fields.toString();
            this.functions += functions.toString();
            this.schematyc += schematycSignals.toString();
            this.includes += additionalIncludes.toString();
            this.reflectFields += reflectFields.toString();
            this.updateFields += updateFields.toString();
            this.schematycFuncs += schematycFuncReg.toString();
            this.schematycSignals += schematycSigReg.toString();
            this.componentFields += componentFields.toString();
            this.constructorFields += constructorFields.toString();
        }

        @Override
        public void visitEventDefinition(EventDefinition eventDef) {
            logger.trace("Generating code for event {}", eventDef.name);
            String upperName = upperName(eventDef.name);
            String type = typeMapper.resolve(eventDef.type);

            String callbackFieldName = "m_" + eventDef.name + "_callbacks";

            // Generate the callback list fields
            {
                ST st = group.getInstanceOf("callbackFieldTemplate");
                st.add("callbackName", callbackFieldName);
                st.add("types", type);
                fields += st.render() + "\n";
                st = group.getInstanceOf("callbackFunctionTemplate");
                st.add("callbackName", upperName);
                st.add("types", type);
                st.add("fieldName", callbackFieldName);
                functions += st.render() + "\n";
            }
            // Generate ApplyComponentUpdate code
            {
                ST st = group.getInstanceOf("updateEvent");
                st.add("spatialEventName", eventDef.name);
                st.add("fieldName", callbackFieldName);
                st.add("signalName", componentUpperName + upperName);
                updateFields += st.render() + '\n';
            }
            // Generate Schematyc code
            {
                ST st = group.getInstanceOf("eventSignalStruct");
                st.add("name", componentUpperName + upperName);
                st.add("type", type);
                st.add("field", eventDef.name);
                schematyc += st.render() + '\n';
                st = group.getInstanceOf("eventSignalReflect");
                st.add("name", componentUpperName + upperName);
                st.add("field", eventDef.name);
                st.add("id", 1);
                st.add("defaultVal", typeMapper.defaultValue(eventDef.type));
                st.add("guid", formatGuid(guidManager.getOrCreateTypeGuid(className + "::" + eventDef.name)));
                schematyc += st.render() + '\n';
                st = group.getInstanceOf("schematycSignal");
                st.add("signalName", "S" + componentUpperName + upperName + "EventSignal");
                schematycSignals += st.render() + "\n";
            }
        }

        private String reflectType(TypeDefinition typeDef) {
            ST st = group.getInstanceOf("reflectTypeCommand");
            st.add("name", upperName(typeDef.name));
            st.add("type", typeDef.outputName());
            st.add("guid", guidManager.getOrCreateTypeGuid(typeDef.qualifiedName));
            st.add("fields", "");
            return st.render() + "\n";
        }

        @Override
        public void visitCommandDefinition(CommandDefinition commandDef) {
            String upperName = upperName(commandDef.name);
            assert(commandDef.requestType.userType != null);
            assert(commandDef.responseType.userType != null);
            logger.trace("Generating header code for {}", commandDef.name);
            TypeDefinition reqType = typeMapper.getTypeDef(commandDef.requestType.userType);
            TypeDefinition resType = typeMapper.getTypeDef(commandDef.responseType.userType);

            String callbackFieldNameResponse = "m_" + commandDef.name + "_rs_callbacks";
            String callbackFieldNameRequest = "m_" + commandDef.name + "_rq_callbacks";

            if (reqType == null || resType == null) {
                logger.error("Failed to generate header code for command {}, request: {}, response: {}", commandDef.name, reqType, resType);
                return;
            }

            ST st = group.getInstanceOf("sendCommandRequestTemplate");
            st.add("name", upperName);
            st.add("type", reqType.outputName());
            st.add("componentClass", componentClassName + "::Commands::" + upperName);
            functions += st.render() + "\n";

            st = group.getInstanceOf("schematycFuncOut");
            st.add("className", className);
            st.add("funcName", "Send" + upperName + "Request");
            st.add("guid", formatGuid(guidManager.getOrCreateCommandFunctionRequest(componentId, commandDef.name)));
            st.add("input", upperName);
            st.add("output", "RequestId");
            schematycFuncs += st.render() + '\n';

            st = group.getInstanceOf("sendCommandResponseTemplate");
            st.add("name", upperName);
            st.add("type", resType.outputName());
            st.add("componentClass", componentClassName + "::Commands::" + upperName);
            functions += st.render() + "\n";

            st = group.getInstanceOf("schematycFuncIn");
            st.add("className", className);
            st.add("funcName", "Send" + upperName + "Response");
            st.add("guid", formatGuid(guidManager.getOrCreateCommandFunctionResponse(componentId, commandDef.name)));
            st.add("input", upperName);
            st.add("input2", "RequestId");
            schematycFuncs += st.render() + '\n';

            st = group.getInstanceOf("callbackFunctionTemplate");
            st.add("callbackName", upperName + "Response");
            st.add("types", resType.outputName() + ", uint32_t");
            st.add("fieldName", callbackFieldNameResponse);
            functions += st.render() + "\n";
            st = group.getInstanceOf("callbackFieldTemplate");
            st.add("callbackName", callbackFieldNameResponse);
            st.add("types", resType.outputName() + ", uint32_t");
            fields += st.render() + "\n";

            st = group.getInstanceOf("callbackFunctionTemplate");
            st.add("callbackName", upperName + "Request");
            st.add("types", reqType.outputName() + ", uint32_t");
            st.add("fieldName", callbackFieldNameRequest);
            functions += st.render() + "\n";
            st = group.getInstanceOf("callbackFieldTemplate");
            st.add("callbackName", callbackFieldNameRequest);
            st.add("types", reqType.outputName()  + ", uint32_t");
            fields += st.render() + "\n";


            st = group.getInstanceOf("signalCommandResponseStructTemplate");
            st.add("name", componentUpperName + upperName);
            st.add("field", commandDef.name);
            st.add("type", resType.outputName());
            schematyc += st.render() + "\n";

            st = group.getInstanceOf("signalCommandResponseReflectType");
            st.add("name", componentUpperName + upperName);
            st.add("field", commandDef.name);
            st.add("type", resType.outputName());
            st.add("guid", formatGuid(guidManager.getOrCreateCommandSignalResponse(componentId, commandDef.name)));
            st.add("id", "0");
            st.add("defaultVal", resType.outputName() + "::Create()");
            schematyc += st.render() + "\n";

            st = group.getInstanceOf("schematycSignal");
            st.add("signalName", "S" + componentUpperName + upperName + "ResponseSignal");
            schematycSignals += st.render() + "\n";

            st = group.getInstanceOf("signalCommandRequestStructTemplate");
            st.add("name", componentUpperName + upperName);
            st.add("field", commandDef.name);
            st.add("type", reqType.outputName());
            schematyc += st.render() + "\n";

            st = group.getInstanceOf("signalCommandRequestReflectType");
            st.add("name", componentUpperName + upperName);
            st.add("field", commandDef.name);
            st.add("type", reqType.outputName());
            st.add("guid", formatGuid(guidManager.getOrCreateCommandSignalRequest(componentId, commandDef.name)));
            st.add("id", "0");
            st.add("defaultVal", reqType.outputName() + "::Create()");
            schematyc += st.render() + "\n";

            st = group.getInstanceOf("schematycSignal");
            st.add("signalName", "S" + componentUpperName + upperName + "RequestSignal");
            schematycSignals += st.render() + "\n";

            st = group.getInstanceOf("receiveCommandResponse");
            st.add("name", upperName);
            st.add("type", resType.outputName());
            st.add("fieldName", callbackFieldNameResponse);
            st.add("signalName", componentUpperName + upperName);
            functions += st.render() + "\n";

            st = group.getInstanceOf("receiveCommandRequest");
            st.add("name", upperName);
            st.add("type", reqType.outputName());
            st.add("fieldName", callbackFieldNameRequest);
            st.add("signalName", componentUpperName + upperName);
            functions += st.render() + "\n";

            st = group.getInstanceOf("addCommandCallback");
            st.add("responseType", componentClassName + "::Commands::" + upperName);
            st.add("requestType", componentClassName + "::Commands::" + upperName);
            st.add("requestName", "On" + upperName + "Request");
            st.add("responseName", "On" + upperName + "Response");
            initialiseBody += st.render() + "\n";
        }

        public void writeToFile(Path outputPath) {
            Path outFilePath = Paths.get(outputPath.toString(), outputFileName);
            File outFile = outFilePath.toFile();

            ST schematycReg = group.getInstanceOf("schematycRegistration");
            schematycReg.add("className", className);
            schematycReg.add("schematycFuncReg", schematycFuncs);
            schematycReg.add("schematycSignalReg", schematycSignals);

            ST st = group.getInstanceOf("baseTemplate");
            st.add("spatialOsHeaderName", componentHeader);
            st.add("name", className);
            st.add("spatialOsComponentName", componentClassName);
            st.add("schematycSignals", schematyc);
            st.add("methods", functions);
            st.add("fields", fields);
            st.add("additionalIncludes", includes);
            st.add("upperName", componentUpperName);
            st.add("componentGuid", formatGuid(guidManager.getOrCreateGuid(componentId)));
            st.add("reflectFields", reflectFields);
            st.add("updateFields", updateFields);
            st.add("initialiseBody", initialiseBody);
            st.add("schematycReg", schematycReg.render());
            st.add("writeComponentFields", componentFields);
            st.add("constructorFields", constructorFields);

            String fileContents = st.render();
            try {
                if (!outFile.getParentFile().exists()) {
                    if (!outFile.getParentFile().mkdirs()) {
                        throw new IOException("Failed to write file: " + outFile.getAbsolutePath() + ", could not create parent directory");
                    }
                }
                logger.info("Writing file {}", outFile);
                FileWriter fw = new FileWriter(outFile);
                fw.write(fileContents);
                fw.close();
            } catch (IOException e) {
                logger.error("Failed to write file to " + outFile, e);
            }
        }
    }
}
