import java.rmi.*;
import java.rmi.server.*;
import java.io.*;
import java.text.*;
import java.util.*;

public class ServicioClimaImpl extends UnicastRemoteObject implements ServicioClima {
    private final List<String> pronosticos;

    public ServicioClimaImpl() throws RemoteException {
        pronosticos = Collections.unmodifiableList(cargarPronosticos());
    }

    public String consultarClima(String fecha) throws RemoteException {
        final ResultadoLocal resultado = new ResultadoLocal();
        Thread hilo = new Thread(new Runnable() {
            public void run() {
                try {
                    if (!fechaValida(fecha)) {
                        resultado.respuesta = "No se pudo interpretar la fecha '" + fecha + "'. Use dd/mm/aaaa.";
                        return;
                    }

                    int dia = extraerDia(fecha);

                    if (dia < 1) {
                        resultado.respuesta = "No se pudo interpretar la fecha '" + fecha + "'. Use dd/mm/aaaa.";
                        return;
                    }

                    if (pronosticos.isEmpty()) {
                        resultado.respuesta = "No hay pronosticos disponibles en el archivo pronosticos.txt.";
                        return;
                    }

                    int indice = (int) (Math.random() * pronosticos.size());
                    resultado.respuesta = "Clima para " + fecha + ": " + pronosticos.get(indice);
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
            throw new RemoteException("La consulta de clima fue interrumpida.", e);
        }

        if (resultado.error != null) {
            throw new RemoteException("Error al procesar la consulta de clima.", resultado.error);
        }

        return resultado.respuesta;
    }

    private int extraerDia(String fecha) {
        String[] partes = fecha.trim().split("/");

        try {
            if (partes.length == 3) {
                return Integer.parseInt(partes[0]);
            }
        }
        catch (NumberFormatException e) {
            return -1;
        }

        return -1;
    }

    private boolean fechaValida(String fecha) {
        if (!fecha.matches("\\d{2}/\\d{2}/\\d{4}")) {
            return false;
        }

        SimpleDateFormat formato = new SimpleDateFormat("dd/MM/yyyy");
        formato.setLenient(false);

        try {
            formato.parse(fecha);
            return true;
        }
        catch (ParseException e) {
            return false;
        }
    }

    private List<String> cargarPronosticos() throws RemoteException {
        List<String> pronosticos = new ArrayList<String>();

        try {
            BufferedReader lector = new BufferedReader(new FileReader(obtenerArchivoLocal("pronosticos.txt")));
            String linea;

            while ((linea = lector.readLine()) != null) {
                if (linea.trim().length() > 0) {
                    pronosticos.add(linea.trim());
                }
            }

            lector.close();
        }
        catch (IOException e) {
            throw new RemoteException("No se pudo leer el archivo pronosticos.txt.", e);
        }

        return pronosticos;
    }

    private File obtenerArchivoLocal(String nombreArchivo) {
        try {
            File base = new File(ServicioClimaImpl.class.getProtectionDomain().getCodeSource().getLocation().toURI());
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
