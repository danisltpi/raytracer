#include "geometry/geometry.h"
#include "sdltemplate.h"
#include "vector"

// Die folgenden Kommentare beschreiben Datenstrukturen und Funktionen
// Die Datenstrukturen und Funktionen die weiter hinten im Text beschrieben sind,
// hängen höchstens von den vorhergehenden Datenstrukturen ab, aber nicht umgekehrt.

// Eine "Kamera", die von einem Augenpunkt aus in eine Richtung senkrecht auf ein Rechteck (das Bild) zeigt.
// Für das Rechteck muss die Auflösung oder alternativ die Pixelbreite und -höhe bekannt sein.
// Für ein Pixel mit Bildkoordinate kann ein Sehstrahl erzeugt werden.


// Für die "Farbe" benötigt man nicht unbedingt eine eigene Datenstruktur.
// Sie kann als Vector3df implementiert werden mit Farbanteil von 0 bis 1.
// Vor Setzen eines Pixels auf eine bestimmte Farbe (z.B. 8-Bit-Farbtiefe),
// kann der Farbanteil mit 255 multipliziert// kann der Farbanteil mit 255 multipliziert  und der Nachkommaanteil verworfen werden.  und der Nachkommaanteil verworfen werden.
using Color = Vector3df;


// Das "Material" der Objektoberfläche mit ambienten, diffusem und reflektiven Farbanteil.

// Ein "Objekt", z.B. eine Kugel oder ein Dreieck, und dem zugehörigen Material der Oberfläche.
// Im Prinzip ein Wrapper-Objekt, das mindestens Material und geometrisches Objekt zusammenfasst.
// Kugel und Dreieck finden Sie in geometry.h/tcc

struct Object {
    Sphere3df sphere;
    Color color;
    bool is_reflecting;
};

struct HitContext {
    Intersection_Context<float, 3u> intersection;
    Object obj;
};


// verschiedene Materialdefinition, z.B. Mattes Schwarz, Mattes Rot, Reflektierendes Weiss, ...
// im wesentlichen Variablen, die mit Konstruktoraufrufen initialisiert werden.


// Die folgenden Werte zur konkreten Objekten, Lichtquellen und Funktionen, wie Lambertian-Shading
// oder die Suche nach einem Sehstrahl für das dem Augenpunkt am nächsten liegenden Objekte,
// können auch zusammen in eine Datenstruktur für die gesammte zu
// rendernde "Szene" zusammengefasst werden.

// Die Cornelbox aufgebaut aus den Objekten
// Am besten verwendet man hier einen std::vector< ... > von Objekten.

// Punktförmige "Lichtquellen" können einfach als Vector3df implementiert werden mit weisser Farbe,
// bei farbigen Lichtquellen müssen die entsprechenden Daten in Objekt zusammengefaßt werden
// Bei mehreren Lichtquellen können diese in einen std::vector gespeichert werden.

// Sie benötigen eine Implementierung von Lambertian-Shading, z.B. als Funktion
// Benötigte Werte können als Parameter übergeben werden, oder wenn diese Funktion eine Objektmethode eines
// Szene-Objekts ist, dann kann auf die Werte teilweise direkt zugegriffen werden.
// Bei mehreren Lichtquellen muss der resultierende diffuse Farbanteil durch die Anzahl Lichtquellen geteilt werden.

// Für einen Sehstrahl aus allen Objekte, dasjenige finden, das dem Augenpunkt am nächsten liegt.
// Am besten einen Zeiger auf das Objekt zurückgeben. Wenn dieser nullptr ist, dann gibt es kein sichtbares Objekt.

// Die rekursive raytracing-Methode. Am besten ab einer bestimmten Rekursionstiefe (z.B. als Parameter übergeben) abbrechen.
Color bg = {0.f, 0.f, 0.f};
Sphere3df sphere1 = {{-10.f, -8.5f, -19.f}, 3.f};
Sphere3df left_wall = {{-10021.0f, 0.f, 0.0f}, 10000.0f};
Sphere3df right_wall = {{10021.0f, 0.f, 0.0f}, 10000.0f};
Sphere3df ceiling = {{0.0f, 10012.f, 0.0f}, 10000.0f};
Sphere3df ground = {{0.0f, -10012.f, 0.0f}, 10000.0f};
Sphere3df back_wall = {{0.f, 0.f, -10030.f}, 10000.0f};


Vector3df light_source = Vector3df{5.f, 11.f, -16.f};

std::vector<Object> objects = {
        Object{sphere1, Color{0.f, 0.f, 1.f}, false},
        Object{left_wall, Color{1.f, 0.f, 0.f}, false},
        Object{right_wall, Color{0.f, 1.f, 0.f}, false},
        Object{ceiling, Color{0.8f, .8f, .8f}, false},
        Object{ground, Color{1.f, 1.f, 1.f}, false},
        Object{back_wall, Color{1.f, 1.f, 1.f}, false},
};

Color lambertian(const HitContext &hc) {
    Intersection_Context<float, 3u> context = hc.intersection;

    Vector3df light_direction = light_source - context.intersection;
    light_direction.normalize();
    float brightness_factor = context.normal * light_direction;
    if (brightness_factor < 0) {
        brightness_factor = 0;
    }

    return brightness_factor * hc.obj.color;
};

HitContext find_nearest_object(const Ray3df &ray) {
    float min_t = INFINITY;
    Intersection_Context<float, 3u> intersection;
    Object visible_object = Object{Sphere3df{Vector3df{0.f, 0.f, 0.f}, 1},
                                   Color{0.f, 0.f, 0.f},
                                   false};
    for (const auto &obj: objects) {
        bool is_intersecting = obj.sphere.intersects(ray, intersection);
        if ((is_intersecting && (intersection.t > 0) && (intersection.t < min_t))) {
            visible_object = obj;
            min_t = intersection.t;
        }
    };
    HitContext hc = {intersection, visible_object};
    return hc;
};


int main() {
    // Ein "Bildschirm", der das Setzen eines Pixels kapselt
    // Der Bildschirm hat eine Auflösung (Breite x Höhe)
    // Kann zur Ausgabe einer PPM-Datei verwendet werden oder
    // mit SDL2 implementiert werden.
    float aspect_ratio = 16.0f / 9.0f;
    int width = 800;
    int height = static_cast<int>(static_cast<float>(width) / aspect_ratio);
    height = (height < 1) ? 1 : height;

    // Camera
    float focal_length = 1.0f;
    float viewport_height = 2.0f;
    float viewport_width = viewport_height *
                           (static_cast<float>(width) / static_cast<float>(height));
    auto camera_center = Vector3df{0.0, 0.0, 0.0};

    // Calculate vectors across horizontal and vertical viewport
    Vector3df viewport_x = Vector3df{viewport_width, 0.0, 0.0};
    Vector3df viewport_y = Vector3df{0.0, -viewport_height, 0.0};

    Vector3df pixel_delta_x = viewport_x * (1.0f / width);
    Vector3df pixel_delta_y = viewport_y * (1.0f / height);

    Vector3df viewport_upper_left = camera_center - Vector3df{0.0, 0.0, focal_length}
                                    - (viewport_x * 0.5f)
                                    - (viewport_y * 0.5f);
    // center of the upper left pixel
    Vector3df pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_x + pixel_delta_y);

    sdltemplate::sdl("Ray Tracer", width, height);
    sdltemplate::loop();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vector3df pixel_center = pixel00_loc
                                     + (static_cast<float>(x) * pixel_delta_x)
                                     + (static_cast<float>(y) * pixel_delta_y);
            Vector3df ray_direction = pixel_center - camera_center;
            Ray3df r{camera_center, ray_direction};
            HitContext hc = find_nearest_object(r);
            Color color = lambertian(hc);
            int ir = static_cast<int>(255 * color[0]);
            int ig = static_cast<int>(255 * color[1]);
            int ib = static_cast<int>(255 * color[2]);
            //std::cout << ir << ' ' << ig << ' ' << ib << " \n";
            sdltemplate::setDrawColor(sdltemplate::createColor(ir, ig, ib, 255));
            sdltemplate::drawPoint(x, y);
        }
    }
    while (sdltemplate::running) {
        sdltemplate::loop();
    }
    return 0;

    // Kamera erstellen

    // Für jede Pixelkoordinate x,y
    //   Sehstrahl für x,y mit Kamera erzeugen
    //   voidFarbe mit raytracing-Methode bestimmen
    //   Beim Bildschirm die Farbe für Pixel x,y, setzten
}

