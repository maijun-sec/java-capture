package zmj.java.capture.model;

import lombok.Data;

import java.util.ArrayList;
import java.util.List;

/**
 * java options, messages used to compile java files
 *
 * @author maijun
 * @since 2021-05-01
 */
@Data
public class JavaOption {
    /**
     * source encoding, default: UTF-8(value of -encoding option)
     */
    private String encoding = "UTF-8";

    /**
     * the java source version, default: 1.8(value of -target option)
     */
    private String source = "1.8";

    /**
     * class path of the dependencies(value of -classpath option)
     */
    private List<String> classpath = new ArrayList<>();

    /**
     * source path of the source files, just like {base_path_root}/src/main/java(value of -sourcepath option)
     */
    private List<String> sourcePath;

    /**
     * all source files(collection all arguments that endswith .java, and check if it's a file)
     */
    private List<String> sourceFiles = new ArrayList<>();

    /**
     * the classes files location(value of -d option)
     */
    private String output;

    /**
     * boot classpath(value of -bootclasspath  option);
     */
    private String bootClasspath;

    /**
     * extdirs(value of -extdirs option)
     */
    private String extDirs;
}
