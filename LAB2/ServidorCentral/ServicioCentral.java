import java.rmi.*;

public interface ServicioCentral extends Remote {
    String consultar(String mensaje) throws RemoteException;
}
