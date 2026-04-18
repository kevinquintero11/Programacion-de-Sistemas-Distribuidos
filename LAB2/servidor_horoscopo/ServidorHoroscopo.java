import java.io.IOException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.ExportException;
import java.rmi.server.UnicastRemoteObject;
import java.util.Locale;

public class ServidorHoroscopo extends UnicastRemoteObject implements HoroscopoRemote {
    private final Config config;

    protected ServidorHoroscopo(Config config) throws RemoteException {
        super();
        this.config = config;
    }

    @Override
    public String consultarHoroscopo(String signo) throws RemoteException {
        String signoNormalizado = signo == null ? "" : signo.trim().toLowerCase(Locale.ROOT);
        String prediccion = config.obtenerPrediccion(signoNormalizado);
        String etiqueta = signoNormalizado.isEmpty() ? "desconocido" : signoNormalizado;
        return "Horoscopo " + etiqueta + ": " + prediccion;
    }

    public static void main(String[] args) {
        try {
            Config config = Config.load("config.conf", "signos.txt");
            ServidorHoroscopo service = new ServidorHoroscopo(config);

            Registry registry;
            try {
                registry = LocateRegistry.createRegistry(config.puertoHoroscopo);
            } catch (ExportException exception) {
                registry = LocateRegistry.getRegistry(config.puertoHoroscopo);
            }

            registry.rebind(HoroscopoRemote.SERVICE_NAME, service);

            System.out.println("Servidor Horoscopo listo.");
            System.out.println("Registry RMI: " + config.puertoHoroscopo);
            System.out.println("Signos cargados: " + config.predicciones.size());
        } catch (IOException exception) {
            System.err.println("No se pudo iniciar el Servidor Horoscopo: " + exception.getMessage());
        }
    }
}
