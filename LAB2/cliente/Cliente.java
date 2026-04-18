import java.io.IOException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class Cliente {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("Entrada esperada: java Cliente <signo> <dd/mm/yyyy>");
            System.err.println("Ejemplo: java Cliente leo 12/08/1999");
            return;
        }

        try {
            Config config = Config.load("config.conf");
            Registry registry = LocateRegistry.getRegistry(config.ipCentral, config.puertoCentral);
            ConsultaCentralRemote remote = (ConsultaCentralRemote) registry.lookup(ConsultaCentralRemote.SERVICE_NAME);

            ConsultaRequest request = new ConsultaRequest(args[0], args[1]);
            ConsultaResponse response = remote.consultar(request);

            System.out.println("Conectado al Servidor Central (" + config.ipCentral + ":" + config.puertoCentral + ")");
            System.out.println("Consulta enviada: Signo=" + request.getSigno() + ", Fecha=" + request.getFecha());
            System.out.println();
            System.out.println("===== RESULTADOS =====");
            System.out.println(response.format());
            if (response.isDesdeCache()) {
                System.out.println("(respuesta obtenida desde cache)");
            }
            System.out.println("======================");
        } catch (IOException exception) {
            System.err.println("No se pudo cargar la configuracion del cliente: " + exception.getMessage());
        } catch (Exception exception) {
            System.err.println("Error al consultar el Servidor Central: " + exception.getMessage());
        }
    }
}
