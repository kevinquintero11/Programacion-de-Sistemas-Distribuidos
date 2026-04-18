import java.io.IOException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.ExportException;
import java.rmi.server.UnicastRemoteObject;

public class ServidorClima extends UnicastRemoteObject implements ClimaRemote {
    private final Config config;

    protected ServidorClima(Config config) throws RemoteException {
        super();
        this.config = config;
    }

    @Override
    public String consultarClima(String fecha) throws RemoteException {
        String fechaNormalizada = fecha == null ? "" : fecha.trim();
        String etiqueta = fechaNormalizada.isEmpty() ? "sin fecha" : fechaNormalizada;
        return "Clima " + etiqueta + ": " + config.obtenerPronostico(fechaNormalizada);
    }

    public static void main(String[] args) {
        try {
            Config config = Config.load("config.conf", "pronosticos.txt");
            ServidorClima service = new ServidorClima(config);

            Registry registry;
            try {
                registry = LocateRegistry.createRegistry(config.puertoClima);
            } catch (ExportException exception) {
                registry = LocateRegistry.getRegistry(config.puertoClima);
            }

            registry.rebind(ClimaRemote.SERVICE_NAME, service);

            System.out.println("Servidor Clima listo.");
            System.out.println("Registry RMI: " + config.puertoClima);
            System.out.println("Pronosticos cargados: " + config.pronosticos.size());
        } catch (IOException exception) {
            System.err.println("No se pudo iniciar el Servidor Clima: " + exception.getMessage());
        }
    }
}
