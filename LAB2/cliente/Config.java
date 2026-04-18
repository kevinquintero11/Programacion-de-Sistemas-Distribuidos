import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class Config {
    final String ipCentral;
    final int puertoCentral;
    final int tamanoBuffer;

    private Config(String ipCentral, int puertoCentral, int tamanoBuffer) {
        this.ipCentral = ipCentral;
        this.puertoCentral = puertoCentral;
        this.tamanoBuffer = tamanoBuffer;
    }

    public static Config load(String path) throws IOException {
        Properties properties = new Properties();
        try (InputStream input = new FileInputStream(path)) {
            properties.load(input);
        }

        return new Config(
            properties.getProperty("ip_servidor_central", "127.0.0.1").trim(),
            getInt(properties, "puerto_servidor_central", 5000),
            getInt(properties, "tamano_buffer", 1024)
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
