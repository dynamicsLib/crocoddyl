import abc
from ode import ODEBase


class DynamicModel(ODEBase):
  """ This abstract class declares virtual methods for defining the system's evolution and its
  derivatives.

  It allows us to define any kind of smooth system dynamcs of the form dx = f(x,u), where n and m
  are the dimension of the state (x) and control (u) vectors, respectively.
  """
  __metaclass__ = abc.ABCMeta

  def __init__(self):
    return None

  @abc.abstractmethod
  def createData(self, n, m):
    """
    :param n: dimension of the state
    :param m: dimension of the control
    """
    from data import DynamicsData
    return DynamicsData(n, m)

  @abc.abstractmethod
  def f(self, data, x, u):
    """ Evaluates the evolution function and stores the result in data.

    :param data: dynamics data
    :param x: system's state
    :param u: control input
    :return: state variation
    """
    pass

  @abc.abstractmethod
  def fx(self, data, x, u):
    """ Evaluates the dynamics Jacobian w.r.t. the state and stores the result in data.

    :param data: dynamics data
    :param x: system's state
    :param u: control input
    :return: Jacobian of the state variation w.r.t the state
    """
    pass

  @abc.abstractmethod
  def fu(self, data, x, u):
    """ Evaluates the dynamics Jacobian w.r.t. the control and stores the result in data.

    :param data: dynamics data
    :param x: system's state
    :param u: control input
    :return: Jacobian of the state variation w.r.t the control
    """
    pass

  @abc.abstractmethod
  def fxx(self, data, x, u):
    """
    Eval the hessian of the dynamics with respect to the state
    :param x:
    :param u:
    :param data:
    :return:
    """
    pass

  # @abc.abstractmethod
  # def fxu(self, data, x, u):
  #   pass

  # @abc.abstractmethod
  # def fuu(self, data, x, u):
  #   pass
