import com.sun.tools.javac.api.JavacTool;
import org.openjdk.btrace.core.BTraceUtils;
import org.openjdk.btrace.core.annotations.BTrace;
import org.openjdk.btrace.core.annotations.Kind;
import org.openjdk.btrace.core.annotations.Location;
import org.openjdk.btrace.core.annotations.OnMethod;
import org.openjdk.btrace.core.types.AnyType;
import zmj.java.capture.util.CommandUtil;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * use btrace to intercept java compile arguments.
 * CAUTION:
 * 1. we don't set package just for convenient to use later;
 * 2. now we only support to capture javac, if want to support other compiler, such as ecj, feel free to add the api
 * 3. use unsafe mode, so we can send all these arguments to the collect module.
 *
 * @author maijun
 * @since 2021-04-30
 */
@BTrace(trusted = true)
public class TraceJavacArgs {
    @OnMethod(
        clazz = "com.sun.tools.javac.Main",
        method = "compile",
        location = @Location(Kind.RETURN)
    )
    public static void traceJavacMain(String[] args) {
        if (args != null && args.length > 0) {
            BTraceUtils.println("======> trace com.sun.tools.javac.Main(String[] args): ");
            List<String> arguments = Stream.of(args).collect(Collectors.toList());
            reportArguments(arguments);
        }
    }

    @OnMethod(
            clazz = "com.sun.tools.javac.Main",
            method = "compile",
            location = @Location(Kind.RETURN)
    )
    public static void traceJavacMain(String[] args, AnyType out) {
        if (args != null && args.length > 0) {
            BTraceUtils.println("======> trace com.sun.tools.javac.Main(String[] args, PrintWriter out): ");
            List<String> arguments = Stream.of(args).collect(Collectors.toList());
            reportArguments(arguments);
        }
    }

    @OnMethod(
            clazz = "com.sun.tools.javac.main.Main",
            method = "compile",
            location = @Location(Kind.RETURN)
    )
    public static void traceJavacMainMain(String[] args) {
        if (args != null && args.length > 0) {
            BTraceUtils.println("======> trace com.sun.tools.javac.main.Main(String[] args): ");
            List<String> arguments = Stream.of(args).collect(Collectors.toList());
            reportArguments(arguments);
        }
    }

    @OnMethod(
            clazz = "com.sun.tools.javac.main.Main",
            method = "compile",
            location = @Location(Kind.RETURN)
    )
    public static void traceJavacMainMain(String[] args, AnyType context) {
        if (args != null && args.length > 0) {
            BTraceUtils.println("======> trace com.sun.tools.javac.main.Main(String[] args, Context context): ");
            List<String> arguments = Stream.of(args).collect(Collectors.toList());
            reportArguments(arguments);
        }
    }

    @OnMethod(
            clazz = "com.sun.tools.javac.main.Main",
            method = "compile",
            location = @Location(Kind.RETURN)
    )
    public static void traceJavacMainMain(String[] args, AnyType context, AnyType fileObjects, AnyType processors) {
        if (args != null && args.length > 0) {
            BTraceUtils.println("======> trace com.sun.tools.javac.main.Main(String[] args, Context context, List<JavaFileObject> fileObjects, Iterable<? extends Processor> processors): ");
            List<String> arguments = Stream.of(args).collect(Collectors.toList());
            reportArguments(arguments);
        }
    }

    @OnMethod(
            clazz = "com.sun.tools.javac.api.JavacTool",
            method = "getTask",
            location = @Location(Kind.RETURN)
    )
    public static void traceJavacTool(AnyType out,
                                      AnyType fileManager,
                                      AnyType diagnosticListener,
                                      Iterable<String> options,
                                      AnyType classes) {
        JavacTool tool = JavacTool.create();
        // Writer var1, JavaFileManager var2,  var3, Iterable<String> var4,  var5,  var6
        BTraceUtils.println("=====> trace JavacTool getTask...");
        handleJavaTool(options);
    }

    @OnMethod(
            clazz = "com.sun.tools.javac.api.JavacTool",
            method = "getTask",
            location = @Location(Kind.RETURN)
    )
    public static void traceJavacTool(AnyType out,
                                      AnyType fileManager,
                                      AnyType diagnosticListener,
                                      AnyType options,
                                      AnyType classes,
                                      AnyType compilationUnits) {
        JavacTool tool = JavacTool.create();
        // Writer var1, JavaFileManager var2,  var3, Iterable<String> var4,  var5,  var6
        BTraceUtils.println("=====> trace JavacTool getTask...");
        List<String> arguments = new ArrayList<>();
        if (options instanceof Iterable) {
            Iterable<String> it = (Iterable) options;
            handleJavaTool(it);
        }
    }

    @OnMethod(
            clazz = "com.sun.tools.javac.api.JavacTaskImpl",
            method = "call",
            location = @Location(Kind.RETURN)
    )
    public static void traceJavacTaskImplCall() {
        BTraceUtils.println("=====> trace JavacTaskImpl call...");
    }

    @OnMethod(
            clazz = "com.sun.tools.javac.api.JavacTaskImpl",
            method = "generate",
            location = @Location(Kind.RETURN)
    )
    public static void traceJavacTaskImplGenerate() {
        BTraceUtils.println("=====> trace JavacTaskImpl generate...");
    }

    private static void handleJavaTool(Iterable<String> options) {
        List<String> arguments = new ArrayList<>();
        Iterator it = ((Iterable) options).iterator();
        while (it.hasNext()) {
            arguments.add(it.next().toString());
        }
        reportArguments(arguments);
    }

    private static void reportArguments(List<String> args) {
        // 1. get environment to get the base path where to tool located
        String toolPath = System.getenv("JAVA_CAPTURE_TOOL_PATH");
        if (toolPath == null) {
            System.err.println("there is no tool environment ENV_TOOL_PATH set");
            return;
        }

        args.add(0, toolPath + File.separator + "bin" + File.separator + "java-capture-collect");
        int result = CommandUtil.execute(args);
        if (result == -1) {
            System.err.println("there are some errors while report the javac arguments.");
        }
    }
}
