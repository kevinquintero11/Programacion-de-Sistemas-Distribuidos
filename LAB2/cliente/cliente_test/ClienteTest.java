import java.io.IOException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadLocalRandom;

public class ClienteTest {
    private static final String[] FECHAS = {
        "10/01/2000",
        "11/02/2001",
        "12/03/2002",
        "13/04/2003",
        "14/05/2004"
    };

    public static void main(String[] args) {
        try {
            Config config = Config.load("config.conf", "signos.txt");
            System.out.println("=== Test de Concurrencia RMI ===");
            System.out.println("Servidor Central: " + config.ipCentral + ":" + config.puertoCentral);
            System.out.println("Hilos: " + config.numHilosTest + " | Consultas por hilo: " + config.consultasPorHilo);

            ExecutorService executor = Executors.newFixedThreadPool(config.numHilosTest);
            CountDownLatch latch = new CountDownLatch(config.numHilosTest);

            for (int i = 0; i < config.numHilosTest; i++) {
                final int id = i + 1;
                executor.submit(new Runnable() {
                    @Override
                    public void run() {
                        ejecutarConsultas(id, config, latch);
                    }
                });
            }

            latch.await();
            executor.shutdown();
            System.out.println();
            System.out.println("TEST FINALIZADO");
        } catch (IOException exception) {
            System.err.println("No se pudo cargar la configuracion del cliente de prueba: " + exception.getMessage());
        } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
            System.err.println("El test fue interrumpido.");
        }
    }

    private static void ejecutarConsultas(int id, Config config, CountDownLatch latch) {
        try {
            Registry registry = LocateRegistry.getRegistry(config.ipCentral, config.puertoCentral);
            ConsultaCentralRemote remote = (ConsultaCentralRemote) registry.lookup(ConsultaCentralRemote.SERVICE_NAME);

            for (int i = 0; i < config.consultasPorHilo; i++) {
                String signo = config.signos.get(ThreadLocalRandom.current().nextInt(config.signos.size()));
                String fecha = FECHAS[ThreadLocalRandom.current().nextInt(FECHAS.length)];

                ConsultaResponse response = remote.consultar(new ConsultaRequest(signo, fecha));

                synchronized (System.out) {
                    System.out.println();
                    System.out.println("[HILO " + id + "] Consulta " + (i + 1));
                    System.out.println("Signo: " + signo + " Fecha: " + fecha);
                    System.out.println("Respuesta:");
                    System.out.println(response.format());
                    if (response.isDesdeCache()) {
                        System.out.println("(respuesta obtenida desde cache)");
                    }
                }

                Thread.sleep(100L);
            }
        } catch (Exception exception) {
            synchronized (System.err) {
                System.err.println("[HILO " + id + "] Error: " + exception.getMessage());
            }
        } finally {
            latch.countDown();
        }
    }
}
