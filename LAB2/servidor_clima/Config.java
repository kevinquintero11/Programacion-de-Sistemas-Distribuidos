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
    final int puertoClima;
    final int tamanoBuffer;
    final List<String> pronosticos;

    private Config(int puertoClima, int tamanoBuffer, List<String> pronosticos) {
        this.puertoClima = puertoClima;
        this.tamanoBuffer = tamanoBuffer;
        this.pronosticos = pronosticos;
    }

    public static Config load(String configPath, String pronosticosPath) throws IOException {
        Properties properties = new Properties();
        try (InputStream input = new FileInputStream(configPath)) {
            properties.load(input);
        }

        List<String> pronosticos = new ArrayList<String>();
        List<String> lineas = Files.readAllLines(Paths.get(pronosticosPath), StandardCharsets.UTF_8);
        for (String linea : lineas) {
            String limpia = linea.trim();
            if (!limpia.isEmpty() && !limpia.startsWith("#")) {
                pronosticos.add(limpia);
            }
        }

        return new Config(
            getInt(properties, "puerto_servidor_clima", 5002),
            getInt(properties, "tamano_buffer", 1024),
            pronosticos
        );
    }

    public String obtenerPronostico(String fecha) {
        if (pronosticos.isEmpty()) {
            return "Pronostico no disponible.";
        }

        String referencia = fecha == null ? "" : fecha.trim();
        int index = Math.floorMod(referencia.hashCode(), pronosticos.size());
        return pronosticos.get(index);
    }

    private static int getInt(Properties properties, String key, int defaultValue) {
        String value = properties.getProperty(key);
        if (value == null) {
            return defaultValue;
        }
        return Integer.parseInt(value.trim());
    }
}
