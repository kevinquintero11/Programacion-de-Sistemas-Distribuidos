import java.rmi.*;
import java.rmi.server.*;
import java.util.*;

public class ServicioCentralImpl extends UnicastRemoteObject implements ServicioCentral {
    private final String ipHoroscopo;
    private final String puertoHoroscopo;
    private final String ipClima;
    private final String puertoClima;
    private final Map<String, String> cache;
    private final HashMap<String, Object> candadosPorClave;

    public ServicioCentralImpl(
        String ipHoroscopo,
        String puertoHoroscopo,
        String ipClima,
        String puertoClima,
        final int tamanoCache
    )
        throws RemoteException {
        this.ipHoroscopo = ipHoroscopo;
        this.puertoHoroscopo = puertoHoroscopo;
        this.ipClima = ipClima;
        this.puertoClima = puertoClima;
        cache = Collections.synchronizedMap(new LinkedHashMap<String, String>(tamanoCache, 0.75f, true) {
            protected boolean removeEldestEntry(Map.Entry<String, String> entradaMasAntigua) {
                return size() > tamanoCache;
            }
        });
        candadosPorClave = new HashMap<String, Object>();
    }

    public String consultar(String mensaje) throws RemoteException {
        String[] partes = mensaje.split("\\|", 2);

        if (partes.length != 2) {
            return "Solicitud invalida. Use el formato signo|fecha.";
        }

        final String signo = partes[0].trim();
        final String fecha = partes[1].trim();

        if (signo.length() == 0 || fecha.length() == 0) {
            return "Solicitud invalida. Debe indicar un signo y una fecha.";
        }

        if (!fechaValida(fecha)) {
            return "Solicitud invalida. La fecha debe tener el formato dd/mm/aaaa.";
        }

        String claveCache = signo + "|" + fecha;
        String respuestaCacheada = obtenerDeCache(claveCache);

        if (respuestaCacheada != null) {
            return respuestaCacheada;
        }

        Object candado = obtenerCandado(claveCache);

        try {
            synchronized (candado) {
                respuestaCacheada = obtenerDeCache(claveCache);

                if (respuestaCacheada != null) {
                    return respuestaCacheada;
                }

                try {
                    final ServicioHoroscopo servicioHoroscopo = (ServicioHoroscopo) Naming.lookup(
                        "//" + ipHoroscopo + ":" + puertoHoroscopo + "/Horoscopo"
                    );
                    final ServicioClima servicioClima = (ServicioClima) Naming.lookup(
                        "//" + ipClima + ":" + puertoClima + "/Clima"
                    );
                    final ResultadoConsulta resultadoHoroscopo = new ResultadoConsulta();
                    final ResultadoConsulta resultadoClima = new ResultadoConsulta();
                    Thread hiloHoroscopo = new Thread(new Runnable() {
                        public void run() {
                            try {
                                resultadoHoroscopo.respuesta = servicioHoroscopo.consultarHoroscopo(signo);
                            }
                            catch (RemoteException e) {
                                resultadoHoroscopo.errorRemoto = e;
                            }
                            catch (Exception e) {
                                resultadoHoroscopo.error = e;
                            }
                        }
                    });
                    Thread hiloClima = new Thread(new Runnable() {
                        public void run() {
                            try {
                                resultadoClima.respuesta = servicioClima.consultarClima(fecha);
                            }
                            catch (RemoteException e) {
                                resultadoClima.errorRemoto = e;
                            }
                            catch (Exception e) {
                                resultadoClima.error = e;
                            }
                        }
                    });

                    hiloHoroscopo.start();
                    hiloClima.start();

                    esperarHilo(hiloHoroscopo, "horoscopo");
                    esperarHilo(hiloClima, "clima");

                    String respuestaHoroscopo = obtenerRespuesta(resultadoHoroscopo, "horoscopo");
                    String respuestaClima = obtenerRespuesta(resultadoClima, "clima");
                    String respuestaFinal = respuestaHoroscopo + " | " + respuestaClima;

                    guardarEnCache(claveCache, respuestaFinal);
                    return respuestaFinal;
                }
                catch (RemoteException e) {
                    throw e;
                }
                catch (Exception e) {
                    throw new RemoteException("No se pudieron consultar los servicios remotos.", e);
                }
            }
        }
        finally {
            synchronized (candadosPorClave) {
                if (candadosPorClave.get(claveCache) == candado) {
                    candadosPorClave.remove(claveCache);
                }
            }
        }
    }

    private Object obtenerCandado(String clave) {
        synchronized (candadosPorClave) {
            Object candado = candadosPorClave.get(clave);

            if (candado == null) {
                candado = new Object();
                candadosPorClave.put(clave, candado);
            }

            return candado;
        }
    }

    private String obtenerDeCache(String clave) {
        synchronized (cache) {
            return cache.get(clave);
        }
    }

    private boolean fechaValida(String fecha) {
        return fecha.matches("\\d{2}/\\d{2}/\\d{4}");
    }

    private void guardarEnCache(String clave, String valor) {
        synchronized (cache) {
            cache.put(clave, valor);
        }
    }

    private void esperarHilo(Thread hilo, String nombreServicio) throws RemoteException {
        try {
            hilo.join();
        }
        catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new RemoteException("La consulta concurrente de " + nombreServicio + " fue interrumpida.", e);
        }
    }

    private String obtenerRespuesta(ResultadoConsulta resultado, String nombreServicio) throws RemoteException {
        if (resultado.errorRemoto != null) {
            throw resultado.errorRemoto;
        }

        if (resultado.error != null) {
            throw new RemoteException("Error al consultar el servicio de " + nombreServicio + ".", resultado.error);
        }

        return resultado.respuesta;
    }

    private static class ResultadoConsulta {
        String respuesta;
        RemoteException errorRemoto;
        Exception error;
    }
}
