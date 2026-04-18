import java.rmi.Remote;
import java.rmi.RemoteException;

public interface ClimaRemote extends Remote {
    String SERVICE_NAME = "ClimaService";

    String consultarClima(String fecha) throws RemoteException;
}
