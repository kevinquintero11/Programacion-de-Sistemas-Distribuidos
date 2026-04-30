import java.rmi.*;
import java.io.*;
import java.util.*;

public class ClienteConcurrente {
    public static void main(String args[]) {
        String archivoConsultas = "consultas.txt";
        String archivoRespuestas = "respuestas_concurrentes.txt";

        if (args.length >= 1) {
            archivoConsultas = args[0];
        }

        if (args.length >= 2) {
            archivoRespuestas = args[1];
        }

        try {
            Properties config = cargarConfiguracion();
            String ipServidorCentral = obtenerPropiedad(config, "ip_servidor_central");
            String puertoServidorCentral = obtenerPropiedad(config, "puerto_servidor_central");
            int tamanoPool = obtenerEnteroPositivo(config, "tamano_pool");

            ServicioCentral srv = (ServicioCentral) Naming.lookup(
                "//" + ipServidorCentral + ":" + puertoServidorCentral + "/Central"
            );

            List<String> consultas = cargarConsultas(archivoConsultas);

            if (consultas.isEmpty()) {
                System.err.println("No hay consultas para procesar en " + archivoConsultas + ".");
                return;
            }

            List<String> respuestas = ejecutarConsultasConcurrentes(srv, consultas, tamanoPool);
            guardarRespuestas(archivoRespuestas, consultas, respuestas);

            System.out.println("Consultas procesadas: " + consultas.size());
            System.out.println("Respuestas guardadas en " + obtenerArchivoLocal(archivoRespuestas).getName());
        }
        catch (Exception e) {
            System.err.println("Excepcion en ClienteConcurrente:");
            e.printStackTrace();
        }
    }

    private static List<String> ejecutarConsultasConcurrentes(
        final ServicioCentral srv,
        List<String> consultas,
        int tamanoPool
    ) throws Exception {
        List<String> respuestas = new ArrayList<String>();

        for (int i = 0; i < consultas.size(); i++) {
            respuestas.add("");
        }

        int inicio = 0;

        while (inicio < consultas.size()) {
            int fin = inicio + tamanoPool;

            if (fin > consultas.size()) {
                fin = consultas.size();
            }

            List<ConsultaClienteRunnable> tareas = new ArrayList<ConsultaClienteRunnable>();
            List<Thread> hilos = new ArrayList<Thread>();

            for (int i = inicio; i < fin; i++) {
                ConsultaClienteRunnable tarea = new ConsultaClienteRunnable(srv, consultas.get(i), i, respuestas);
                Thread hilo = new Thread(tarea);

                tareas.add(tarea);
                hilos.add(hilo);
                hilo.start();
            }

            for (int i = 0; i < hilos.size(); i++) {
                hilos.get(i).join();

                if (tareas.get(i).getError() != null) {
                    throw tareas.get(i).getError();
                }
            }

            inicio = fin;
        }

        return respuestas;
    }

    private static List<String> cargarConsultas(String nombreArchivo) throws IOException {
        List<String> consultas = new ArrayList<String>();
        BufferedReader lector = new BufferedReader(new FileReader(obtenerArchivoLocal(nombreArchivo)));

        try {
            String linea;

            while ((linea = lector.readLine()) != null) {
                String consulta = linea.trim();

                if (consulta.length() > 0) {
                    consultas.add(consulta);
                }
            }
        }
        finally {
            lector.close();
        }

        return consultas;
    }

    private static void guardarRespuestas(String nombreArchivo, List<String> consultas, List<String> respuestas)
        throws IOException {
        BufferedWriter escritor = new BufferedWriter(new FileWriter(obtenerArchivoLocal(nombreArchivo)));

        try {
            for (int i = 0; i < consultas.size(); i++) {
                escritor.write("Consulta " + (i + 1) + ": " + consultas.get(i));
                escritor.newLine();
                escritor.write("Respuesta " + (i + 1) + ": " + respuestas.get(i));
                escritor.newLine();
                escritor.newLine();
            }
        }
        finally {
            escritor.close();
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
            File base = new File(ClienteConcurrente.class.getProtectionDomain().getCodeSource().getLocation().toURI());
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

    private static class ConsultaClienteRunnable implements Runnable {
        private final ServicioCentral servicio;
        private final String consulta;
        private final int indice;
        private final List<String> respuestas;
        private Exception error;

        private ConsultaClienteRunnable(ServicioCentral servicio, String consulta, int indice, List<String> respuestas) {
            this.servicio = servicio;
            this.consulta = consulta;
            this.indice = indice;
            this.respuestas = respuestas;
        }

        public void run() {
            try {
                respuestas.set(indice, servicio.consultar(consulta));
            }
            catch (Exception e) {
                error = e;
            }
        }

        private Exception getError() {
            return error;
        }
    }
}
