import java.rmi.*;
import java.io.*;
import java.util.*;

public class ClienteConsulta {
    public static void main(String args[]) {
        if (args.length < 1) {
            System.err.println("Uso: ClienteConsulta signo|fecha [...]");
            return;
        }

        try {
            Properties config = cargarConfiguracion();
            String ipServidorCentral = obtenerPropiedad(config, "ip_servidor_central");
            String puertoServidorCentral = obtenerPropiedad(config, "puerto_servidor_central");

            ServicioCentral srv = (ServicioCentral) Naming.lookup(
                "//" + ipServidorCentral + ":" + puertoServidorCentral + "/Central"
            );

            for (int i = 0; i < args.length; i++) {
                System.out.println(srv.consultar(args[i]));
            }
        }
        catch (RemoteException e) {
            System.err.println("Error de comunicacion: " + e.toString());
        }
        catch (Exception e) {
            System.err.println("Excepcion en ClienteConsulta:");
            e.printStackTrace();
        }
    }

    private static Properties cargarConfiguracion() throws IOException {
        Properties config = new Properties();
        FileInputStream archivo = new FileInputStream(obtenerArchivoLocal("config.conf"));
        config.load(archivo);
        archivo.close();
        return config;
    }

    private static File obtenerArchivoLocal(String nombreArchivo) {
        try {
            File base = new File(ClienteConsulta.class.getProtectionDomain().getCodeSource().getLocation().toURI());
            return new File(base, nombreArchivo);
        }
        catch (Exception e) {
            return new File(nombreArchivo);
        }
    }

    private static String obtenerPropiedad(Properties config, String clave) {
        String valor = config.getProperty(clave);

        if (valor == null || valor.trim().length() == 0) {
            throw new IllegalArgumentException("Falta la propiedad '" + clave + "' en config.conf.");
        }

        return valor.trim();
    }
}
