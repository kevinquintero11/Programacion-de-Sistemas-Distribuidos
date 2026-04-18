import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

public class Config {
    final String ipCentral;
    final int puertoCentral;
    final int tamanoBuffer;
    final int numHilosTest;
    final int consultasPorHilo;
    final List<String> signos;

    private Config(
        String ipCentral,
        int puertoCentral,
        int tamanoBuffer,
        int numHilosTest,
        int consultasPorHilo,
        List<String> signos
    ) {
        this.ipCentral = ipCentral;
        this.puertoCentral = puertoCentral;
        this.tamanoBuffer = tamanoBuffer;
        this.numHilosTest = numHilosTest;
        this.consultasPorHilo = consultasPorHilo;
        this.signos = signos;
    }

    public static Config load(String configPath, String signosPath) throws IOException {
        Properties properties = new Properties();
        try (InputStream input = new FileInputStream(configPath)) {
            properties.load(input);
        }

        List<String> signos = new ArrayList<String>();
        for (String linea : Files.readAllLines(Paths.get(signosPath), StandardCharsets.UTF_8)) {
            String limpia = linea.trim();
            if (!limpia.isEmpty() && !limpia.startsWith("#")) {
                signos.add(limpia);
            }
        }

        if (signos.isEmpty()) {
            throw new IOException("No se encontraron signos de prueba en " + signosPath);
        }

        return new Config(
            properties.getProperty("ip_servidor_central", "127.0.0.1").trim(),
            getInt(properties, "puerto_servidor_central", 5000),
            getInt(properties, "tamano_buffer", 1024),
            Math.max(1, getInt(properties, "num_hilos", 10)),
            Math.max(1, getInt(properties, "consultas_por_hilo", 5)),
            signos
        );
    }

    private static int getInt(Properties properties, String key, int defaultValue) {
        String value = properties.getProperty(key);
        if (value == null) {
            return defaultValue;
        }
        return Integer.parseInt(value.trim());
    }
}
