import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;

public class Config {
    final int puertoHoroscopo;
    final int tamanoBuffer;
    final Map<String, String> predicciones;

    private Config(int puertoHoroscopo, int tamanoBuffer, Map<String, String> predicciones) {
        this.puertoHoroscopo = puertoHoroscopo;
        this.tamanoBuffer = tamanoBuffer;
        this.predicciones = predicciones;
    }

    public static Config load(String configPath, String signosPath) throws IOException {
        Properties properties = new Properties();
        try (InputStream input = new FileInputStream(configPath)) {
            properties.load(input);
        }

        Map<String, String> predicciones = new LinkedHashMap<String, String>();
        List<String> lineas = Files.readAllLines(Paths.get(signosPath), StandardCharsets.UTF_8);
        for (String linea : lineas) {
            String limpia = linea.trim();
            if (limpia.isEmpty() || limpia.startsWith("#")) {
                continue;
            }

            int separador = limpia.indexOf('|');
            if (separador < 0) {
                continue;
            }

            String signo = limpia.substring(0, separador).trim().toLowerCase(Locale.ROOT);
            String prediccion = limpia.substring(separador + 1).trim();
            predicciones.put(signo, prediccion);
        }

        return new Config(
            getInt(properties, "puerto_servidor_horoscopo", 5001),
            getInt(properties, "tamano_buffer", 1024),
            predicciones
        );
    }

    public String obtenerPrediccion(String signo) {
        String clave = signo == null ? "" : signo.trim().toLowerCase(Locale.ROOT);
        String prediccion = predicciones.get(clave);
        return prediccion == null ? "Signo no reconocido." : prediccion;
    }

    private static int getInt(Properties properties, String key, int defaultValue) {
        String value = properties.getProperty(key);
        if (value == null) {
            return defaultValue;
        }
        return Integer.parseInt(value.trim());
    }
}
