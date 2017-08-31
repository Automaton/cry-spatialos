package uk.automaton.crysos.json.type;

import com.google.gson.annotations.SerializedName;
import uk.automaton.crysos.json.SourceReference;
import uk.automaton.crysos.json.TypeReference;

import java.util.LinkedList;
import java.util.List;

public class FieldDefinition {

    public enum FieldType {
        SINGULAR, OPTION, LIST, MAP
    }

    public static class OptionType {

        @Override
        public String toString() {
            return "OptionType{" +
                    "valueType=" + valueType +
                    '}';
        }

        @SerializedName("valueType")
        public TypeReference valueType;

    }

    public static class ListType {

        @Override
        public String toString() {
            return "ListType{" +
                    "valueType=" + valueType +
                    '}';
        }

        @SerializedName("valueType")
        public TypeReference valueType;

    }

    public static class MapType {

        @Override
        public String toString() {
            return "MapType{" +
                    "keyType=" + keyType +
                    ", valueType=" + valueType +
                    '}';
        }

        @SerializedName("keyType")
        public TypeReference keyType;

        @SerializedName("valueType")
        public TypeReference valueType;

    }

    @SerializedName("sourceReference")
    public SourceReference sourceReference;

    @SerializedName("name")
    public String name;

    @SerializedName("number")
    public int number;

    @SerializedName("singularType")
    public TypeReference singularType;

    @SerializedName("optionType")
    public OptionType optionType;

    @SerializedName("listType")
    public ListType listType;

    @SerializedName("mapType")
    public MapType mapType;


    public FieldType getFieldType() {
        if (singularType != null) return FieldType.SINGULAR;
        if (optionType != null) return FieldType.OPTION;
        if (listType != null) return FieldType.LIST;
        if (mapType != null) return FieldType.MAP;
        throw new RuntimeException("No field type");
    }

    public List<TypeReference> getReferences() {
        List<TypeReference> typeRefs = new LinkedList<>();
        switch (getFieldType()) {
            case LIST:
                typeRefs.add(listType.valueType);
                break;
            case SINGULAR:
                typeRefs.add(singularType);
                break;
            case OPTION:
                typeRefs.add(optionType.valueType);
                break;
            case MAP:
                typeRefs.add(mapType.keyType);
                typeRefs.add(mapType.valueType);
                break;
        }
        return typeRefs;
    }

    @Override
    public String toString() {
        return "FieldDefinition{" +
                "sourceReference=" + sourceReference +
                ", name='" + name + '\'' +
                ", number=" + number +
                ", singularType=" + singularType +
                ", optionType=" + optionType +
                ", listType=" + listType +
                ", mapType=" + mapType +
                '}';
    }
}
