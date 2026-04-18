import java.rmi.Remote;
import java.rmi.RemoteException;

public interface ConsultaCentralRemote extends Remote {
    String SERVICE_NAME = "ConsultaCentralService";

    ConsultaResponse consultar(ConsultaRequest request) throws RemoteException;
}
