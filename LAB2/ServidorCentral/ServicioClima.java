import java.rmi.*;

public interface ServicioClima extends Remote {
    String consultarClima(String fecha) throws RemoteException;
}
