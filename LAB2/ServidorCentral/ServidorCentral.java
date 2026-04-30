import java.rmi.*;
import java.rmi.registry.*;
import java.io.*;
import java.util.*;

public class ServidorCentral {
    public static void main(String args[]) {
        try {
            Properties config = cargarConfiguracion();
            String ipServidorCentral = obtenerPropiedad(config, "ip_servidor_central");
            String puertoServidorCentral = obtenerPropiedad(config, "puerto_servidor_central");
            String ipServidorHoroscopo = obtenerPropiedad(config, "ip_servidor_horoscopo");
            String puertoServidorHoroscopo = obtenerPropiedad(config, "puerto_servidor_horoscopo");
            String ipServidorClima = obtenerPropiedad(config, "ip_servidor_clima");
            String puertoServidorClima = obtenerPropiedad(config, "puerto_servidor_clima");
            int tamanoCache = obtenerEnteroPositivo(config, "tamano_cache");

            LocateRegistry.createRegistry(Integer.parseInt(puertoServidorCentral));
            ServicioCentralImpl srv = new ServicioCentralImpl(
                ipServidorHoroscopo,
                puertoServidorHoroscopo,
                ipServidorClima,
                puertoServidorClima,
                tamanoCache
            );
            Naming.rebind("rmi://" + ipServidorCentral + ":" + puertoServidorCentral + "/Central", srv);
            System.out.println("Servidor central listo en " + ipServidorCentral + ":" + puertoServidorCentral);
        }
        catch (RemoteException e) {
            System.err.println("Error de comunicacion: " + e.toString());
            System.exit(1);
        }
        catch (Exception e) {
            System.err.println("Excepcion en ServidorCentral:");
            e.printStackTrace();
            System.exit(1);
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
            File base = new File(ServidorCentral.class.getProtectionDomain().getCodeSource().getLocation().toURI());
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

    private static int obtenerEnteroPositivo(Properties config, String clave) {
        String valor = obtenerPropiedad(config, clave);
        int numero = Integer.parseInt(valor);

        if (numero <= 0) {
            throw new IllegalArgumentException("La propiedad '" + clave + "' debe ser mayor que cero.");
        }

        return numero;
    }
}
