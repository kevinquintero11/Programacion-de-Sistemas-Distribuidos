import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class Config {
    final String ipHoroscopo;
    final String ipClima;
    final int puertoCentral;
    final int puertoHoroscopo;
    final int puertoClima;
    final int tamanoCache;
    final int tamanoBuffer;

    private Config(
        String ipHoroscopo,
        String ipClima,
        int puertoCentral,
        int puertoHoroscopo,
        int puertoClima,
        int tamanoCache,
        int tamanoBuffer
    ) {
        this.ipHoroscopo = ipHoroscopo;
        this.ipClima = ipClima;
        this.puertoCentral = puertoCentral;
        this.puertoHoroscopo = puertoHoroscopo;
        this.puertoClima = puertoClima;
        this.tamanoCache = tamanoCache;
        this.tamanoBuffer = tamanoBuffer;
    }

    public static Config load(String path) throws IOException {
        Properties properties = new Properties();
        try (InputStream input = new FileInputStream(path)) {
            properties.load(input);
        }

        return new Config(
            getString(properties, "ip_servidor_horoscopo", "127.0.0.1"),
            getString(properties, "ip_servidor_clima", "127.0.0.1"),
            getInt(properties, "puerto_servidor_central", 5000),
            getInt(properties, "puerto_servidor_horoscopo", 5001),
            getInt(properties, "puerto_servidor_clima", 5002),
            getInt(properties, "tamano_cache", 100),
            getInt(properties, "tamano_buffer", 1024)
        );
    }

    private static String getString(Properties properties, String key, String defaultValue) {
        return properties.getProperty(key, defaultValue).trim();
    }

    private static int getInt(Properties properties, String key, int defaultValue) {
        String value = properties.getProperty(key);
        if (value == null) {
            return defaultValue;
        }
        return Integer.parseInt(value.trim());
    }
}
