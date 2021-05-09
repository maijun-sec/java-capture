package zmj.java.capture.util;

import lombok.extern.slf4j.Slf4j;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.List;

/**
 * command execute utils
 *
 * @author maijun
 * @since 2021-05-01
 */
@Slf4j
public class CommandUtil {
    /**
     * execute command
     *
     * @param args command arguments
     * @return is run successfully
     */
    public static int execute(List<String> args) {
        int exitVal = -1;

        ProcessBuilder pb = new ProcessBuilder(args);
        Process process = null;
        try {
            process = pb.start();
            process.getOutputStream().close();
            StreamGobbler outputGobbler = new StreamGobbler(process.getInputStream());
            StreamGobbler errorGobbler = new StreamGobbler(process.getErrorStream());
            outputGobbler.start();
            errorGobbler.start();

            exitVal = process.waitFor();
            outputGobbler.join();
            errorGobbler.join();
        } catch (IOException e) {
            log.error("can't run command for {}", e);
        } catch (InterruptedException e) {
            log.error("interrupted: {}", e);
        } finally {
            if (process != null) {
                process.destroy();
            }
        }

        return exitVal;
    }

    @Slf4j
    static class StreamGobbler extends Thread {
        private InputStream inputStream;

        public StreamGobbler(InputStream inputStream) {
            this.inputStream = inputStream;
        }

        @Override
        public void run() {
            try (InputStreamReader isr = new InputStreamReader(inputStream);
                 BufferedReader br = new BufferedReader(isr)) {
                br.lines().forEach(System.out::println);
            } catch (IOException e) {
                log.error("there are some error while getting command output message for {}", e);
            }
        }
    }
}
