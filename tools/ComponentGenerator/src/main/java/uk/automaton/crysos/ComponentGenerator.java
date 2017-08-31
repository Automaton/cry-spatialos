package uk.automaton.crysos;

import com.google.gson.Gson;
import com.google.gson.JsonSyntaxException;
import com.google.gson.reflect.TypeToken;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.stringtemplate.v4.ST;
import org.stringtemplate.v4.STGroupFile;
import uk.automaton.crysos.json.ASTFile;
import uk.automaton.crysos.json.ComponentDefinition;
import uk.automaton.crysos.json.TypeDefinition;

import java.io.*;
import java.lang.reflect.Type;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;
import java.util.stream.Collectors;

public class ComponentGenerator {

    private final File inputDirectory;
    private final File outputDirectory;
    private final File cacheDirectory;
    private final Gson gson;

    private Map<String, ASTFile> astFiles;
    private TypeMapper typeMapper;
    private GuidManager guidManager;

    private static final Logger logger = LoggerFactory.getLogger(ComponentGenerator.class);
    private Map<String, Long> dateModifiedCache;
    private Map<String, Long> dateModified;

    public ComponentGenerator(File inputDir, File outputDir, File cacheDirectory) {
        this.inputDirectory = inputDir;
        this.outputDirectory = outputDir;
        this.cacheDirectory = cacheDirectory;
        this.dateModifiedCache = new HashMap<>();
        this.dateModified = new HashMap<>();
        gson = new Gson();
        typeMapper = new TypeMapper();
        astFiles = new HashMap<>();
    }

    private void readInput() {
        Queue<File> queue = new LinkedList<>();
        queue.add(inputDirectory);
        final Path inPath = Paths.get(inputDirectory.toURI()).normalize();
        while (!queue.isEmpty()) {
            File file = queue.poll();
            if (file.isDirectory()) {
                File[] files = file.listFiles();
                if (files == null) continue;
                for (File f : files) {
                    if (f.isDirectory() || f.getName().endsWith(".json")) {
                        queue.add(f);
                    }
                }
            } else if (file.getName().endsWith(".json")) {
                Path absPath = Paths.get(file.getAbsolutePath()).normalize();
                Path relPath = inPath.relativize(absPath);
                logger.trace("Found JSON AST: {}", relPath.toString());
                dateModified.put(relPath.toString(), file.lastModified());
                try {
                    FileReader reader = new FileReader(file);
                    ASTFile object = gson.fromJson(reader, ASTFile.class);
                    astFiles.put(relPath.toString(), object);
                } catch (FileNotFoundException e) {
                    logger.error("Failed to find file: " + absPath.toString(), e);
                } catch (JsonSyntaxException e) {
                    logger.error("Failed to parse file: " + relPath.toString(), e);
                }
            }
        }
        logger.info("Loaded {} JSON AST files", astFiles.size());
    }

    private void parseTypes() {
        for (ASTFile ast : astFiles.values()) {
            for (TypeDefinition typeDef : ast.typeDefinitions) {
                typeMapper.addType(typeDef, ast.canonicalName.replace(".schema", ".h"));
            }
        }
        for (ASTFile ast : astFiles.values()) {
            for (ComponentDefinition cd : ast.componentDefinitions) {
                int componentId = cd.id;
                // Components will always have their own type - so wont be built in
                String qualifiedName = cd.dataDefinition.getType();
                assert(cd.dataDefinition.builtInType == null);
                TypeDefinition typeDef = typeMapper.getTypeDef(qualifiedName);
                if (typeDef == null) {
                    logger.warn("Failed to resolve type ({}) for component ID: {}", qualifiedName, componentId);
                    continue;
                }
                typeMapper.addTypeId(componentId, typeDef, cd);
            }
        }
    }

    private void generateCode() {
        if (dateModified.equals(dateModifiedCache)) {
            logger.info("No changes detected.");
            return;
        }
        CodeGenerator codeGen = new CodeGenerator(typeMapper, guidManager);
        List<CodeGenerator.GenerateSchematyc> schematycGen = new LinkedList<>();
        for (ASTFile ast : astFiles.values()) {
            try {
                codeGen.generateHeader(ast, Paths.get(outputDirectory.toURI()).toRealPath());
            } catch (IOException e) {
                logger.error("Failed to write file", e);
            }
            schematycGen.add(codeGen.generateSchematyc(ast));
        }
        generateTypesFile(schematycGen);
    }

    private void generateTypesFile(List<CodeGenerator.GenerateSchematyc> schematycGen) {
        try {
            Path path = Paths.get(outputDirectory.toURI()).toRealPath();
            File header = new File(path.toFile(), "Types.h");
            File src = new File(path.toFile(), "Types.cpp");

            STGroupFile group = new STGroupFile("templates/SchematycTemplate.stg");
            {
                ST st = group.getInstanceOf("typeFileHeader");
                st.add("includes", schematycGen.stream().map(CodeGenerator.GenerateSchematyc::getIncludes).distinct().collect(Collectors.joining("\n")));
                st.add("signatures", schematycGen.stream().map(CodeGenerator.GenerateSchematyc::getSignatures).collect(Collectors.joining("\n")));
                st.add("regFuncs", schematycGen.stream().map(CodeGenerator.GenerateSchematyc::getFunctionReg).collect(Collectors.joining("\n")));
                String headerSrc = st.render();
                FileWriter writer = new FileWriter(header);
                writer.write(headerSrc);
                writer.close();
            }
//            {
//                ST st = group.getInstanceOf("typeFileSource");
//                st.add("regFuncs", schematycGen.stream().map(CodeGenerator.GenerateSchematyc::getFunctionReg).collect(Collectors.joining("\n")));
//                st.add("functionBodies", schematycGen.stream().map(CodeGenerator.GenerateSchematyc::getFunctionBodies).collect(Collectors.joining("\n")));
//                String implSrc = st.render();
//                FileWriter writer = new FileWriter(src);
//                writer.write(implSrc);
//                writer.close();
//            }
        } catch (IOException e) {
            logger.error("Failed to write types files", e);
        }
    }

    private void initialiseCache() {
        guidManager = new GuidManager(new File(cacheDirectory, "components.json"), new File(cacheDirectory,  "types.json"), typeMapper);
        guidManager.initialise();
        File modifiedCache = new File(cacheDirectory, "date.json");
        if (modifiedCache.exists()) {
            try {
                Type type = new TypeToken<Map<String, Long>>() {}.getType();
                dateModifiedCache = gson.fromJson(new FileReader(modifiedCache), type);
            } catch (FileNotFoundException e) {
                logger.error("Failed to find date cache file: " + modifiedCache.getAbsolutePath(), e);
            }
        } else {
            dateModifiedCache = new HashMap<>();
        }
    }

    private void writeCache() {
        guidManager.writeFile();
        File modifiedCache = new File(cacheDirectory, "date.json");
        try {
            FileWriter writer = new FileWriter(modifiedCache);
            gson.toJson(dateModified, writer);
            writer.close();
        } catch (IOException e) {
            logger.error("Failed to write date cache", e);
        }
    }

    private void run() {
        initialiseCache();
        readInput();
        parseTypes();
        generateCode();
        writeCache();
    }

    public static void main(String[] args) {
        if (args.length < 2) {
            logger.error("Not enough args, specify input_dir and output_dir");
            System.exit(1);
            return;
        }

        String indir = args[0];
        String outdir = args[1];
        String cachedir = ".cache";
        if (args.length > 2) {
            cachedir = args[2];
        }

        File inputDir = new File(indir);
        File outputDir = new File(outdir);
        File cacheDir = new File(cachedir);

        if (outputDir.exists()) {
            if (!outputDir.isDirectory()) {
                logger.error("Output directory is not a directory: {}", outputDir.getAbsolutePath());
                System.exit(2);
                return;
            }
        } else {
            if (!outputDir.mkdirs()) {
                logger.error("Failed to create output directory");
                System.exit(2);
                return;
            }
        }

        if (cacheDir.exists()) {
            if (!cacheDir.isDirectory()) {
                logger.error("Cache directory is not a directory: {}", cacheDir.getAbsolutePath());
                System.exit(3);
                return;
            }
        } else {
            if (!cacheDir.mkdirs()) {
                logger.error("Failed to create cache directory");
                System.exit(3);
                return;
            }
        }

        try {
            logger.info("Input directory: " + inputDir.getCanonicalPath());
            logger.info("Output directory: " + outputDir.getCanonicalPath());
            logger.info("Cache directory: " + cacheDir.getCanonicalPath());
        } catch (IOException e) {
            logger.error("Failed to get input & output directories", e);
            System.exit(4);
            return;
        }

        ComponentGenerator generator = new ComponentGenerator(inputDir, outputDir, cacheDir);
        generator.run();
    }

}
