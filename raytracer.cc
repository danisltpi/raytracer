#include "geometry/geometry.h"
#include "sdltemplate.h"

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
class Material {

};


// Ein "Objekt", z.B. eine Kugel oder ein Dreieck, und dem zugehörigen Material der Oberfläche.
// Im Prinzip ein Wrapper-Objekt, das mindestens Material und geometrisches Objekt zusammenfasst.
// Kugel und Dreieck finden Sie in geometry.h/tcc


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
Sphere3df sphere1 = Sphere3df{Vector3df{0.0f, 0.0f, -1.0f}, 0.5f};

float hit_sphere(const Sphere3df &sphere, const Ray3df &ray) {
    return sphere.intersects(ray);
}


Vector3df ray_color(const Ray3df &ray) {
    float t = hit_sphere(sphere1, ray);
    if (t > 0.0) {
        Vector3df normal = (ray.origin + t * ray.direction) - Vector3df{0, 0, -1};
        normal.normalize();
        return 0.5f * Color{normal[0] + 1, normal[1] + 1, normal[2] + 1};
    }
    Vector3df dir = ray.direction;
    dir.normalize();
    t = 0.5f * (dir[1] + 1.0f);
    return (1.0f - t) * Color{1.0, 1.0, 1.0} + t * Color{0.5, 0.7, 1.0};
}


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

    Vector3df upper_left_corner{2.0, -1.0, -1.0};
    Vector3df horizontal{4.0, 0.0, 0.0};
    Vector3df vertical{0.0, 2.0, 0.0};
    Vector3df origin{0.0, 0.0, 0.0};
    sdltemplate::sdl("Ray Tracer", width, height);
    sdltemplate::loop();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vector3df pixel_center = pixel00_loc
                                     + (static_cast<float>(x) * pixel_delta_x)
                                     + (static_cast<float>(y) * pixel_delta_y);
            Vector3df ray_direction = pixel_center - camera_center;
            Ray3df r{camera_center, ray_direction};

            Vector3df col = ray_color(r);
            int ir = int(255.999 * col[0]);
            int ig = int(255.999 * col[1]);
            int ib = int(255.999 * col[2]);
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

