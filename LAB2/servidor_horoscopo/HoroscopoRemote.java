import java.rmi.Remote;
import java.rmi.RemoteException;

public interface HoroscopoRemote extends Remote {
    String SERVICE_NAME = "HoroscopoService";

    String consultarHoroscopo(String signo) throws RemoteException;
}
