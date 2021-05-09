package zmj.java.capture;

import com.fasterxml.jackson.databind.ObjectMapper;
import lombok.extern.slf4j.Slf4j;
import zmj.java.capture.constant.Constant;
import zmj.java.capture.model.JavaOption;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * collect all java options
 *
 * @author maijun
 * @since 2021-05-01
 */
@Slf4j
public class Main {
    public static void main(String[] args) {
        // 1. check arguments
        // 1.1 arguments input
        if (args == null || args.length == 0) {
            log.warn("there is no arguments input, will directly return.");
            return;
        }

        // 1.2 environment
        String outputDir = System.getenv(Constant.ENV_OUTPUT);
        if (outputDir == null) {
            log.error("no output directory set, break.");
            return;
        }

        // 2. extract all java options
        JavaOption javaOption = extractJavaOption(args);

        // 3. collect all message(copy java file, jar file, class file) and record arguments.
        // TODO now we just record all arguments. not remove the duplicate arguments.
        File outputFolder = new File(outputDir);
        if (!outputFolder.exists() || outputFolder.isFile()) {
            outputFolder.mkdirs();
        }

        ObjectMapper mapper = new ObjectMapper();
        try {
            mapper.writeValue(new File(outputDir + File.separator + UUID.randomUUID().toString() + ".txt"), javaOption);
        } catch (IOException e) {
            log.error("can't write javac options to file.");
        }
    }

    private static JavaOption extractJavaOption(String[] args) {
        JavaOption javaOption = new JavaOption();
        for (int i = 0; i < args.length - 1; i ++) {
            if (Constant.JAVAC_OPTION_D.equals(args[i])) {
                javaOption.setOutput(args[i + 1]);
            } else if (Constant.JAVAC_OPTION_ENCODING.equals(args[i])) {
                javaOption.setEncoding(args[i + 1]);
            } else if (Constant.JAVAC_OPTION_TARGET.equals(args[i])) {
                javaOption.setSource(args[i + 1]);
            } else if (Constant.JAVAC_OPTION_SOURCEPATH.equals(args[i])) {
                javaOption.setSourcePath(extractEntry(args[i + 1]));
            } else if (Constant.JAVAC_OPTION_BOOT_CLASSPATH.equals(args[i])) {
                javaOption.setBootClasspath(args[i + 1]);
            } else if (Constant.JAVAC_OPTION_EXTDIRS.equals(args[i])) {
                javaOption.setExtDirs(args[i + 1]);
            } else if (Constant.JAVAC_OPTON_CLASSPATH.equals(args[i]) || Constant.JAVAC_OPTION_CP.equals(args[i])) {
                javaOption.setClasspath(extractEntry(args[i + 1]));
            } else if (args[i].endsWith(".java")) {
                javaOption.getSourceFiles().add(args[i]);
            }
        }

        if (args[args.length - 1] != null && args[args.length - 1].endsWith(".java")) {
            javaOption.getSourceFiles().add(args[args.length - 1]);
        }

        return javaOption;
    }

    private static List<String> extractEntry(String arg) {
        if (arg == null) {
            return new ArrayList<>();
        }

        return Stream.of(arg.split(":|;")).collect(Collectors.toList());
    }
}
