import java.io.IOException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.ExportException;
import java.rmi.server.UnicastRemoteObject;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ServidorCentral extends UnicastRemoteObject implements ConsultaCentralRemote {
    private final Config config;
    private final ExecutorService executor;
    private final Map<String, ConsultaResponse> cache;

    protected ServidorCentral(Config config) throws RemoteException {
        super();
        this.config = config;
        this.executor = Executors.newCachedThreadPool();
        this.cache = Collections.synchronizedMap(new LinkedHashMap<String, ConsultaResponse>(16, 0.75f, true) {
            @Override
            protected boolean removeEldestEntry(Map.Entry<String, ConsultaResponse> eldest) {
                return size() > ServidorCentral.this.config.tamanoCache;
            }
        });
    }

    @Override
    public ConsultaResponse consultar(ConsultaRequest request) throws RemoteException {
        if (request == null) {
            throw new RemoteException("La solicitud no puede ser nula.");
        }

        ConsultaResponse respuestaCache = cache.get(request.cacheKey());
        if (respuestaCache != null) {
            return new ConsultaResponse(respuestaCache.getHoroscopo(), respuestaCache.getClima(), true);
        }

        CompletableFuture<String> horoscopoFuture = CompletableFuture.supplyAsync(
            () -> consultarHoroscopo(request.getSigno()),
            executor
        );
        CompletableFuture<String> climaFuture = CompletableFuture.supplyAsync(
            () -> consultarClima(request.getFecha()),
            executor
        );

        try {
            ConsultaResponse response = new ConsultaResponse(horoscopoFuture.get(), climaFuture.get(), false);
            cache.put(request.cacheKey(), response);
            return response;
        } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
            throw new RemoteException("La consulta fue interrumpida.", exception);
        } catch (ExecutionException exception) {
            throw new RemoteException("Fallo la consulta a los servicios secundarios.", exception.getCause());
        }
    }

    private String consultarHoroscopo(String signo) {
        try {
            Registry registry = LocateRegistry.getRegistry(config.ipHoroscopo, config.puertoHoroscopo);
            HoroscopoRemote remote = (HoroscopoRemote) registry.lookup(HoroscopoRemote.SERVICE_NAME);
            return remote.consultarHoroscopo(signo);
        } catch (Exception exception) {
            return "Horoscopo " + signo + ": Error al consultar el servicio remoto.";
        }
    }

    private String consultarClima(String fecha) {
        try {
            Registry registry = LocateRegistry.getRegistry(config.ipClima, config.puertoClima);
            ClimaRemote remote = (ClimaRemote) registry.lookup(ClimaRemote.SERVICE_NAME);
            return remote.consultarClima(fecha);
        } catch (Exception exception) {
            return "Clima " + fecha + ": Error al consultar el servicio remoto.";
        }
    }

    private void shutdown() {
        executor.shutdownNow();
    }

    public static void main(String[] args) {
        try {
            Config config = Config.load("config.conf");
            ServidorCentral service = new ServidorCentral(config);

            Runtime.getRuntime().addShutdownHook(new Thread(service::shutdown));

            Registry registry;
            try {
                registry = LocateRegistry.createRegistry(config.puertoCentral);
            } catch (ExportException exception) {
                registry = LocateRegistry.getRegistry(config.puertoCentral);
            }

            registry.rebind(ConsultaCentralRemote.SERVICE_NAME, service);

            System.out.println("Servidor Central listo.");
            System.out.println("Registry RMI: " + config.puertoCentral);
            System.out.println(
                "Servicios remotos configurados: horoscopo=" + config.ipHoroscopo + ":" + config.puertoHoroscopo
                    + ", clima=" + config.ipClima + ":" + config.puertoClima
            );
        } catch (IOException exception) {
            System.err.println("No se pudo cargar la configuracion del Servidor Central: " + exception.getMessage());
        }
    }
}
