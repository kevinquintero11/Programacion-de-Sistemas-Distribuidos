import java.io.Serializable;

public class ConsultaResponse implements Serializable {
    private static final long serialVersionUID = 1L;

    private final String horoscopo;
    private final String clima;
    private final boolean desdeCache;

    public ConsultaResponse(String horoscopo, String clima, boolean desdeCache) {
        this.horoscopo = horoscopo;
        this.clima = clima;
        this.desdeCache = desdeCache;
    }

    public String getHoroscopo() {
        return horoscopo;
    }

    public String getClima() {
        return clima;
    }

    public boolean isDesdeCache() {
        return desdeCache;
    }

    public String format() {
        return horoscopo + System.lineSeparator() + clima;
    }
}
