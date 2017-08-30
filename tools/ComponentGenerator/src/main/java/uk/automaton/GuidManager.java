package uk.automaton;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import uk.automaton.json.ComponentDefinition;
import uk.automaton.json.TypeDefinition;
import uk.automaton.json.component.CommandDefinition;
import uk.automaton.json.type.FieldDefinition;

import java.io.*;
import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

public class GuidManager {

    private final File guidFile;
    private final File typeFile;
    private final Gson gson;
    private Map<Integer, ComponentGuids> guids;
    private Map<String, TypeGuids> typeGuids;
    private TypeMapper typeMapper;
    private static final Logger logger = LoggerFactory.getLogger(GuidManager.class);

    public GuidManager(final File guidFile, final File typeFile, final TypeMapper mapper) {
        this.guidFile = guidFile;
        this.typeFile = typeFile;
        this.gson = new Gson();
        this.typeMapper = mapper;
    }

    public void initialise() {
        if (guidFile.exists()) {
            logger.info("Reading component cache file: {}", guidFile.getAbsolutePath());
            try {
                Type type = new TypeToken<Map<Integer, ComponentGuids>>() {}.getType();
                guids = gson.fromJson(new FileReader(guidFile), type);
            } catch (FileNotFoundException e) {
                logger.error("Failed to find GUID file: " + guidFile.getAbsolutePath(), e);
            }
        } else {
            guids = new HashMap<>();
        }
        if (typeFile.exists()) {
            logger.info("Reading type cache file: {}", typeFile.getAbsolutePath());
            try {
                Type type = new TypeToken<Map<String, TypeGuids>>() {}.getType();
                typeGuids = gson.fromJson(new FileReader(typeFile), type);
            } catch (FileNotFoundException e) {
                logger.error("Failed to find GUID file: " + typeFile.getAbsolutePath(), e);
            }
        } else {
            typeGuids = new HashMap<>();
        }
    }

    public void writeFile() {
        logger.trace("Writing GUID component cache file: {}", guidFile);
        try {
            FileWriter fw = new FileWriter(guidFile);
            fw.write(gson.toJson(guids));
            fw.close();
        } catch (Exception e) {
            logger.error("Failed to write GUID component cache file", e);
        }
        logger.trace("Writing GUID type cache file: {}", typeFile);
        try {
            FileWriter fw = new FileWriter(typeFile);
            fw.write(gson.toJson(typeGuids));
            fw.close();
        } catch (Exception e) {
            logger.error("Failed to write GUID type cache file", e);
        }
    }

    public UUID getOrCreateTypeGuid(String type) {
        TypeGuids tg = typeGuids.get(type);
        if (tg == null) {
            tg = createGuids(type);
        }
        return tg.typeGuid;
    }

    private TypeGuids createGuids(String type) {
        TypeGuids guids = new TypeGuids();
        guids.typeGuid = UUID.randomUUID();
        typeGuids.put(type, guids);
        return guids;
    }

    public UUID getOrCreateGuid(int componentId) {
        ComponentGuids cg = guids.get(componentId);
        if (cg == null) {
            cg = createGuids(componentId);
        }
        return cg.componentGuid;
    }

    public UUID getOrCreateGuidSignal(int componentId, int number) {
        ComponentGuids cg = guids.get(componentId);
        if (cg == null) {
            cg = createGuids(componentId);
        }
        UUID id = cg.signalGuids.get(number);
        if (id == null) {
            cg.signalGuids.put(number, id = UUID.randomUUID());
        }
        return id;
    }

    public UUID getOrCreateCommandSignalResponse(int componentId, String commandName) {
        ComponentGuids cg = guids.get(componentId);
        if (cg == null) {
            cg = createGuids(componentId);
        }
        UUID id = cg.commandResponseSignalGuids.get(commandName);
        if (id == null) {
            cg.commandResponseSignalGuids.put(commandName, id = UUID.randomUUID());
        }
        return id;
    }

    public UUID getOrCreateCommandSignalRequest(int componentId, String commandName) {
        ComponentGuids cg = guids.get(componentId);
        if (cg == null) {
            cg = createGuids(componentId);
        }
        UUID id = cg.commandRequestSignalGuids.get(commandName);
        if (id == null) {
            cg.commandRequestSignalGuids.put(commandName, id = UUID.randomUUID());
        }
        return id;
    }
    public ComponentGuids getGuids(int componentId) {
        return guids.get(componentId);
    }

    public ComponentGuids createGuids(int componentId) {
        if (guids.containsKey(componentId)) {
            logger.warn("Recreating GUIDs for a component that already has them");
        }
        ComponentGuids guids = new ComponentGuids();
        generateUUIDs(componentId, guids);
        this.guids.put(componentId, guids);
        return guids;
    }

    private void generateUUIDs(int componentId, ComponentGuids cg) {
        logger.info("Generating new GUIDs for component {}", componentId);
        cg.componentGuid = UUID.randomUUID();
        cg.signalGuids = new HashMap<>();
        cg.functionGuids = new HashMap<>();
        cg.commandFunctionRequestGuids = new HashMap<>();
        cg.commandFunctionResponseGuids = new HashMap<>();
        cg.commandRequestSignalGuids = new HashMap<>();
        cg.commandResponseSignalGuids = new HashMap<>();
        TypeDefinition typeDef = typeMapper.getById(componentId);
        if (typeDef == null) {
            logger.warn("Failed to find type definition for component ID {}", componentId);
            return;
        }
        for (FieldDefinition fd : typeDef.fieldDefinitions) {
            cg.signalGuids.put(fd.number, UUID.randomUUID());
            cg.functionGuids.put(fd.number, UUID.randomUUID());
        }
        ComponentDefinition cd = typeMapper.getComponent(componentId);
        if (cd == null) {
            logger.warn("Failed to find component definition for component ID {}", componentId);
            return;
        }
        for (CommandDefinition command : cd.commandDefinitions) {
            cg.commandRequestSignalGuids.put(command.name, UUID.randomUUID());
            cg.commandResponseSignalGuids.put(command.name, UUID.randomUUID());
            cg.commandFunctionRequestGuids.put(command.name, UUID.randomUUID());
            cg.commandFunctionResponseGuids.put(command.name, UUID.randomUUID());
        }
    }

    public UUID getOrCreateGuidFunction(int componentId, int number) {
        ComponentGuids cg = guids.get(componentId);
        if (cg == null) {
            cg = createGuids(componentId);
        }
        UUID id = cg.functionGuids.get(number);
        if (id == null) {
            cg.functionGuids.put(number, id = UUID.randomUUID());
        }
        return id;
    }

    public UUID getOrCreateCommandFunctionRequest(int componentId, String name) {
        ComponentGuids cg = guids.get(componentId);
        if (cg == null) {
            cg = createGuids(componentId);
        }
        UUID id = cg.commandFunctionRequestGuids.get(name);
        if (id == null) {
            cg.commandFunctionRequestGuids.put(name, id = UUID.randomUUID());
        }
        return id;
    }

    public UUID getOrCreateCommandFunctionResponse(int componentId, String name) {
        ComponentGuids cg = guids.get(componentId);
        if (cg == null) {
            cg = createGuids(componentId);
        }
        UUID id = cg.commandFunctionResponseGuids.get(name);
        if (id == null) {
            cg.commandResponseSignalGuids.put(name, id = UUID.randomUUID());
        }
        return id;
    }

    static class ComponentGuids {
        public UUID componentGuid;
        public Map<Integer, UUID> signalGuids;
        public Map<Integer, UUID> functionGuids;
        public Map<String, UUID> commandRequestSignalGuids;
        public Map<String, UUID> commandResponseSignalGuids;
        public Map<String, UUID> commandFunctionRequestGuids;
        public Map<String, UUID> commandFunctionResponseGuids;
    }

    static class TypeGuids {
        public UUID typeGuid;
    }
}
