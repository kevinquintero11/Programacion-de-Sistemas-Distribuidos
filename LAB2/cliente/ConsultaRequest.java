import java.io.Serializable;
import java.util.Locale;

public class ConsultaRequest implements Serializable {
    private static final long serialVersionUID = 1L;

    private final String signo;
    private final String fecha;

    public ConsultaRequest(String signo, String fecha) {
        this.signo = normalizarSigno(signo);
        this.fecha = limpiar(fecha);
    }

    private static String normalizarSigno(String valor) {
        return limpiar(valor).toLowerCase(Locale.ROOT);
    }

    private static String limpiar(String valor) {
        return valor == null ? "" : valor.trim();
    }

    public String getSigno() {
        return signo;
    }

    public String getFecha() {
        return fecha;
    }

    public String cacheKey() {
        return signo + "|" + fecha;
    }
}
