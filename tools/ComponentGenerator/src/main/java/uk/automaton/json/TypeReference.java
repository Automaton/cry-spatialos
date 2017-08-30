package uk.automaton.json;

import com.google.gson.annotations.SerializedName;

public class TypeReference {

    @SerializedName("sourceReference")
    public SourceReference sourceReference;

    @SerializedName("builtInType")
    public String builtInType;

    @SerializedName("userType")
    public String userType;

    public String getType() {
        if (builtInType == null) return userType;
        return builtInType;
    }

    public boolean isBuiltIn() {
        return builtInType != null;
    }

    @Override
    public String toString() {
        return "TypeReference{" +
                "sourceReference=" + sourceReference +
                ", builtInType='" + builtInType + '\'' +
                ", userType='" + userType + '\'' +
                '}';
    }
}
