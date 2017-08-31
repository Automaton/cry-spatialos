package uk.automaton.crysos;

import com.google.common.collect.ImmutableMap;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import uk.automaton.crysos.json.ComponentDefinition;
import uk.automaton.crysos.json.TypeDefinition;
import uk.automaton.crysos.json.TypeReference;
import uk.automaton.crysos.json.type.FieldDefinition;

import java.util.*;

public class TypeMapper {

    private final Map<String, TypeDefinition> types;
    private final Map<Integer, TypeDefinition> idTypes;
    private final Map<Integer, ComponentDefinition> components;
    private final Map<TypeDefinition, String> typeLocations;
    private static final ImmutableMap<String, BuiltInType> builtInTypes;
    private static final Logger logger = LoggerFactory.getLogger(TypeMapper.class);

    static {
        builtInTypes = ImmutableMap.<String, BuiltInType>builder()
                .put("uint32", new BuiltInType("uint32_t", "0", BuiltInType.Type.NUMBER))
                .put("bool", new BuiltInType("bool", "false", BuiltInType.Type.BOOL))
                .put("uint64", new BuiltInType("uint64_t", "0", BuiltInType.Type.NUMBER))
                .put("sint32", new BuiltInType("int32_t", "0", BuiltInType.Type.NUMBER))
                .put("sint64", new BuiltInType("int64_t", "0", BuiltInType.Type.NUMBER))
                .put("fixed32", new BuiltInType("uint32_t", "0", BuiltInType.Type.NUMBER))
                .put("fixed64", new BuiltInType("uint64_t", "0", BuiltInType.Type.NUMBER))
                .put("sfixed32", new BuiltInType("int32_t", "0", BuiltInType.Type.NUMBER))
                .put("sfixed64", new BuiltInType("int64_t", "0", BuiltInType.Type.NUMBER))
                .put("float", new BuiltInType("float", "0", BuiltInType.Type.NUMBER))
                .put("double", new BuiltInType("double", "0", BuiltInType.Type.NUMBER))
                .put("string", new BuiltInType("std::string", "\"\"", BuiltInType.Type.STRING))
                .put("bytes", new BuiltInType("const char *", "nullptr", BuiltInType.Type.ARRAY))
                .put("EntityId", new BuiltInType("worker::EntityId", "0", BuiltInType.Type.NUMBER))
                .build();
    }

    public TypeMapper() {
        types = new HashMap<>();
        idTypes = new HashMap<>();
        typeLocations = new HashMap<>();
        components = new HashMap<>();
    }

    public void addType(TypeDefinition typeDef, String outputFile) {
        if (types.containsKey(typeDef.qualifiedName)) {
            logger.warn("{} was already defined as a type - being overwritten", typeDef.qualifiedName);
        }
        types.put(typeDef.qualifiedName, typeDef);
        typeLocations.put(typeDef, outputFile);
    }

    public void addTypeId(int componentId, TypeDefinition typeDef, ComponentDefinition compDef) {
        if (idTypes.containsKey(componentId)) {
            logger.warn("{} was already defined as a ID {} type - being overwritten", typeDef.qualifiedName, componentId);
        }
        idTypes.put(componentId, typeDef);
        components.put(componentId, compDef);
    }

    public TypeDefinition getTypeDef(String qualifiedName) {
        return types.get(qualifiedName);
    }

    public static boolean isBuiltIn(String name) {
        return builtInTypes.containsKey(name);
    }

    public static BuiltInType getBuiltIn(String name) {
        return builtInTypes.get(name);
    }

    public TypeDefinition getById(int id) {
        return idTypes.get(id);
    }

    public String getLocation(TypeDefinition typeDef) {
        return typeLocations.get(typeDef);
    }

    public ComponentDefinition getComponent(int id)  {
        return components.get(id);
    }

    public String defaultValue(FieldDefinition fieldDef) {
        String fieldType = getOutputType(fieldDef);
        switch (fieldDef.getFieldType()) {
            case SINGULAR:
                return defaultValue(fieldDef.singularType);
            case OPTION:
                return fieldType + "()";
            case LIST:
                return fieldType + "()";
            case MAP:
                return fieldType + "()";
        }
        return "nullptr";
    }

    public String defaultValue(TypeReference ref) {
        if (ref.isBuiltIn()) {
            BuiltInType builtIn = getBuiltIn(ref.builtInType);
            if (builtIn == null) {
                logger.error("Failed to get default value for built in type {}", ref.builtInType);
                return "nullptr";
            }
            return builtIn.defaultValue;
        }
        return resolve(ref) + "::Create()";
    }


    public String resolve(TypeReference ref) {
        if (ref.isBuiltIn()) {
            BuiltInType fieldType = getBuiltIn(ref.builtInType);
            if (fieldType == null) {
                logger.warn("Failed to find built in type mapping for {} - falling back on no mapping", ref.builtInType);
                return ref.builtInType;
            }
            return fieldType.output;
        } else {
            TypeDefinition def = getTypeDef(ref.userType);
            if (def == null) {
                return null;
            }
            return def.qualifiedName.replace(".", "::");
        }
    }

    public String getOutputType(FieldDefinition fieldDef) {
        String fieldType;
        switch (fieldDef.getFieldType()) {
            case SINGULAR: {
                TypeReference ref = fieldDef.singularType;
                fieldType = resolve(ref);
            }
            break;
            case OPTION: {
                FieldDefinition.OptionType ot = fieldDef.optionType;
                TypeReference ref = ot.valueType;
                String optionType = resolve(ref);
                fieldType = "worker::Option<" + optionType + ">";
            }
            break;
            case LIST: {
                FieldDefinition.ListType lt = fieldDef.listType;
                TypeReference ref = lt.valueType;
                String listType = resolve(ref);
                fieldType = "worker::List<" + listType + ">";
            }
            break;
            case MAP: {
                FieldDefinition.MapType mt = fieldDef.mapType;
                TypeReference valType = mt.valueType;
                TypeReference keyType = mt.keyType;
                String value = resolve(valType);
                String key = resolve(keyType);
                fieldType = "worker::Map<" + key + ", " + value + ">";
            }
            break;
            default: {
                fieldType = null;
                break;
            }
        }
        return fieldType;
    }

    public boolean isComponentType(TypeDefinition typeDef) {
        return idTypes.containsValue(typeDef);
    }

    static class BuiltInType {
        enum Type {
            NUMBER, BOOL, STRING, ARRAY
        }
        public BuiltInType(String output, String defaultValue, Type type) {
            this.output = output;
            this.defaultValue = defaultValue;
            this.type = type;
        }

        public final String output;
        public final String defaultValue;
        public final Type type;
    }
}
