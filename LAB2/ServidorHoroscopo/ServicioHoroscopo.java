import java.rmi.*;

public interface ServicioHoroscopo extends Remote {
    String consultarHoroscopo(String signo) throws RemoteException;
}
