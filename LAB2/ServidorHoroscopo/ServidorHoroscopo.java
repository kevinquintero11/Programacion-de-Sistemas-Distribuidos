import java.rmi.*;
import java.rmi.registry.*;
import java.io.*;
import java.util.*;

public class ServidorHoroscopo {
    public static void main(String args[]) {
        try {
            Properties config = cargarConfiguracion();
            String ipServidorHoroscopo = obtenerPropiedad(config, "ip_servidor_horoscopo");
            String puertoServidorHoroscopo = obtenerPropiedad(config, "puerto_servidor_horoscopo");
            obtenerPropiedad(config, "ip_servidor_central");
            obtenerPropiedad(config, "puerto_servidor_central");

            LocateRegistry.createRegistry(Integer.parseInt(puertoServidorHoroscopo));
            ServicioHoroscopoImpl srv = new ServicioHoroscopoImpl();
            Naming.rebind("rmi://" + ipServidorHoroscopo + ":" + puertoServidorHoroscopo + "/Horoscopo", srv);
            System.out.println("Servidor de horoscopo listo en " + ipServidorHoroscopo + ":" + puertoServidorHoroscopo);
        }
        catch (RemoteException e) {
            System.err.println("Error de comunicacion: " + e.toString());
            System.exit(1);
        }
        catch (Exception e) {
            System.err.println("Excepcion en ServidorHoroscopo:");
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
            File base = new File(ServidorHoroscopo.class.getProtectionDomain().getCodeSource().getLocation().toURI());
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
