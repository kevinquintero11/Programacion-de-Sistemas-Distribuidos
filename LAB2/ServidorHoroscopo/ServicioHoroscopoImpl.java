import java.rmi.*;
import java.rmi.server.*;
import java.text.Normalizer;
import java.io.*;
import java.util.*;

public class ServicioHoroscopoImpl extends UnicastRemoteObject implements ServicioHoroscopo {
    private final Map<String, String> horoscopos;

    public ServicioHoroscopoImpl() throws RemoteException {
        horoscopos = Collections.unmodifiableMap(cargarHoroscopos());
    }

    public String consultarHoroscopo(String signo) throws RemoteException {
        final ResultadoLocal resultado = new ResultadoLocal();
        Thread hilo = new Thread(new Runnable() {
            public void run() {
                try {
                    String signoNormalizado = normalizar(signo);
                    String respuesta = horoscopos.get(signoNormalizado);

                    if (respuesta == null) {
                        resultado.respuesta = "Horoscopo no disponible para el signo '" + signo + "'.";
                        return;
                    }

                    resultado.respuesta = respuesta;
                }
                catch (Exception e) {
                    resultado.error = e;
                }
            }
        });
        hilo.start();

        try {
            hilo.join();
        }
        catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new RemoteException("La consulta de horoscopo fue interrumpida.", e);
        }

        if (resultado.error != null) {
            throw new RemoteException("Error al procesar la consulta de horoscopo.", resultado.error);
        }

        return resultado.respuesta;
    }

    private String normalizar(String texto) {
        String limpio = Normalizer.normalize(texto.trim().toLowerCase(), Normalizer.Form.NFD);
        return limpio.replaceAll("\\p{M}", "");
    }

    private Map<String, String> cargarHoroscopos() throws RemoteException {
        Map<String, String> horoscopos = new HashMap<String, String>();

        try {
            BufferedReader lector = new BufferedReader(new FileReader(obtenerArchivoLocal("signos.txt")));
            String linea;

            while ((linea = lector.readLine()) != null) {
                String[] partes = linea.split("\\|", 2);

                if (partes.length == 2) {
                    horoscopos.put(normalizar(partes[0]), partes[1].trim());
                }
            }

            lector.close();
        }
        catch (IOException e) {
            throw new RemoteException("No se pudo leer el archivo signos.txt.", e);
        }

        return horoscopos;
    }

    private File obtenerArchivoLocal(String nombreArchivo) {
        try {
            File base = new File(ServicioHoroscopoImpl.class.getProtectionDomain().getCodeSource().getLocation().toURI());
            return new File(base, nombreArchivo);
        }
        catch (Exception e) {
            return new File(nombreArchivo);
        }
    }

    private static class ResultadoLocal {
        String respuesta;
        Exception error;
    }
}
